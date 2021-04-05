#!/usr/bin/env node
import { readFileSync } from 'fs';
import * as Module from 'module';
import { EOL } from 'os';
import * as path from 'path';

const metacall_require = (Module.prototype as any).require;
const node_require = (Module.prototype as any).node_require;
const node_cache = (Module.prototype as any).node_cache || require.cache;

/** Unpatch in order to load TypeScript */
if (node_require) {
	(Module.prototype as any).require = node_require;
}

/** Native node require */
import * as ts from 'typescript';

/** Patch again */
if (node_require) {
	(Module.prototype as any).require = metacall_require;
}

type anyF = (...args: any[]) => any;

const noop = () => ({});

type MetacallExport = {
	ptr: anyF;
	signature: string[];
	types: string[];
	ret: string;
	async: boolean;
};

type MetacallExports = Record<string, MetacallExport>;

type MetacallHandle = Record<string, unknown>;

const discoverTypes = new Map<string, MetacallExports>();

/** Logging util */
const log = process.env.METACALL_DEBUG ? console.log : noop;

/** Util: Wraps a function in try / catch and possibly logs */
const safe = <F extends anyF, Def>(f: F, def: Def) =>
	(...args: Parameters<F>): ReturnType<F> | Def => {
		try {
			log(f.name, '<-', ...args);
			const res = f(...args);
			log(f.name, '->', res);
			return res;
		} catch (err) {
			console.log(`Exception in ${f.name}`, err);
			return def;
		}
	};

const wrapFunctionExport = (e: unknown) =>
	typeof e === 'function' ? { [e.name]: e } : e as Record<string, unknown>;

const defaultCompilerOptions = {
	target: ts.ScriptTarget.ES2017,
	module: ts.ModuleKind.CommonJS,
	lib: ['lib.es2017'],
};

const getCompilerOptions = () => {
	const configFile = ts.findConfigFile(
		process.cwd(),
		ts.sys.fileExists,
		'tsconfig.json',
	);
	if (!configFile) {
		return defaultCompilerOptions;
	}
	const { config, error } = ts.readConfigFile(configFile, ts.sys.readFile);
	if (error) {
		return defaultCompilerOptions;
	}
	const { errors, options } = ts.convertCompilerOptionsFromJson(
		config,
		process.cwd(),
		configFile,
	);
	if (errors.length > 0) {
		return defaultCompilerOptions;
	}
	return options;
};

const getMetacallExportTypes = (
	p: ts.Program,
	cb: (sourceFile: ts.SourceFile, metacallType: MetacallExports) => void =
		() => { },
) => {
	const exportTypes: MetacallExports = {};
	const sourceFiles = p.getRootFileNames().map((name) =>
		[name, p.getSourceFile(name)] as const
	);
	for (const [fileName, sourceFile] of sourceFiles) {
		if (!sourceFile) {
			// TODO: Implement better exception handling
			console.log(`Error: Failed to load ${fileName}, file not found.`);
			continue;
		}
		const c = p.getTypeChecker();
		const sym = c.getSymbolAtLocation(sourceFile);
		const moduleExports = sym ? c.getExportsOfModule(sym) : [];
		for (const e of moduleExports) {
			const metacallType =
				(exportTypes[e.name] = exportTypes[e.name] || ({} as MetacallExport));
			const exportType = c.getTypeOfSymbolAtLocation(e, sourceFile);

			const callSignatures = exportType.getCallSignatures();
			if (callSignatures.length === 0) {
				continue;
			}
			for (const signature of callSignatures) {
				const parameters = signature.getParameters();
				metacallType.signature = parameters.map((p) => p.name);
				metacallType.types = parameters.map((p) =>
					c.typeToString(c.getTypeOfSymbolAtLocation(p, p.valueDeclaration))
				);
				const returnType = signature.getReturnType();
				metacallType.ret = c.typeToString(returnType);
				const flags = ts.getCombinedModifierFlags(e.valueDeclaration);
				metacallType.async = Boolean(flags & ts.ModifierFlags.Async);
			}
		}
		cb(sourceFile, exportTypes);
	}
	return exportTypes;
};

/** Loads a TypeScript file from disk */
export const load_from_file = safe(function load_from_file(paths: string[]) {
	const result: MetacallHandle = {};
	const p = ts.createProgram(paths, getCompilerOptions());
	getMetacallExportTypes(p, (sourceFile, exportTypes) => {
		p.emit(sourceFile, (fileName, data) => {
			const m = new Module(fileName);
			(m as any)._compile(data, fileName);
			const wrappedExports = wrapFunctionExport(m.exports);
			for (const [name, handle] of Object.entries(exportTypes)) {
				handle.ptr = wrappedExports[name] as anyF;
			}
			discoverTypes.set(fileName, {
				...(discoverTypes.get(fileName) ?? {}),
				...exportTypes,
			});
			result[fileName] = wrappedExports;
		});
	});

	return Object.keys(result).length !== 0 ? result : null;
}, null);

/** Loads a TypeScript file from memory */
export const load_from_memory = safe(
	function load_from_memory(name: string, data: string) {
		const compilerOptions = getCompilerOptions();
		const transpileOutput = ts.transpileModule(data, { compilerOptions });
		const extName = `${name}.ts`;
		const target = compilerOptions.target ?? defaultCompilerOptions.target;
		const p = ts.createProgram([extName], getCompilerOptions(), {
			fileExists: (fileName) => fileName === extName,
			getCanonicalFileName: (fileName) => fileName,
			getCurrentDirectory: ts.sys.getCurrentDirectory,
			getDefaultLibFileName: ts.getDefaultLibFileName,
			getNewLine: () => EOL,
			getSourceFile: (fileName) => {
				if (fileName === extName) {
					return ts.createSourceFile(fileName, data, target);
				}
				if (fileName.endsWith('.d.ts')) {
					try {
						const tsPath = path.join(path.dirname(require.resolve('typescript')), fileName);
						return ts.createSourceFile(
							fileName,
							readFileSync(tsPath, 'utf8'),
							target,
						);
					} catch (err) {
						return ts.createSourceFile(
							fileName,
							readFileSync(fileName, 'utf8'),
							target,
						);
					}
				}
			},
			readFile: (fileName) => fileName === extName ? data : undefined,
			useCaseSensitiveFileNames: () => true,
			writeFile: () => { },
		});
		const exportTypes = getMetacallExportTypes(p);
		const m = new Module(name);
		(m as any)._compile(transpileOutput.outputText, name);
		const result: MetacallHandle = {
			[name]: wrapFunctionExport(m.exports),
		};
		for (const [n, handle] of Object.entries(exportTypes)) {
			handle.ptr = (result[name] as Record<string, anyF>)[n];
		}
		discoverTypes.set(name, exportTypes);
		return result;
	},
	null,
);

/** Unloads a TypeScript file using handle returned from load_from_file / load_from_memory */
export const clear = safe(function clear(handle: Record<string, string>) {
	const names = Object.getOwnPropertyNames(handle);

	for (let i = 0; i < names.length; ++i) {
		const p = names[i];
		const absolute = path.resolve(__dirname, p);

		discoverTypes.delete(p);

		if (node_cache[absolute]) {
			delete node_cache[absolute];
		}
	}
}, undefined as void);

/** Returns type information about exported functions from a given handle */
export const discover = safe(function discover(handle: Record<string, any>) {
	const result = Object.keys(handle)
		.reduce<MetacallExports>(
			(acc, k) => ({ ...acc, ...discoverTypes.get(k) ?? {} }),
			{},
		);
	return result;
}, {});

/** Unimplemented */
export const execution_path = noop;
/** Unimplemented */
export const load_from_package = noop;
/** Unimplemented */
export const test = noop;
/** Unimplemented */
export const initialize = noop;
/** Unimplemented */
export const destroy = noop;
