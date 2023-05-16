#!/usr/bin/env node
import { readFileSync } from 'fs';
import * as Module from 'module';
import { EOL } from 'os';
import * as path from 'path';

const monkey_patch_require = (Module.prototype as any).require;
const node_require = (Module.prototype as any).node_require;
const node_cache = (Module.prototype as any).node_cache || require.cache;
const node_resolve = (Module.prototype as any).node_resolve || require.resolve;

/** Unpatch in order to load TypeScript */
if (node_require) {
	(Module.prototype as any).require = node_require;
}

/** Native node require */
import * as ts from 'typescript';

/** Define the extensions for requiring with TypeScript */
['ts', 'tsx', 'jsx'].forEach(ext => {
	/* If we require a TypeScript file from NodeJS, probably we do not need introspection data */
	(Module as any)._extensions[`.${ext}`] = (module: Module) => {
		const exp = load_from_file([module.filename], false);
		if (exp !== null) {
			const values = Object.values(exp);
			if (values.length === 1) {
				module.exports = values[0];
			}
		}
	}
});

/** Patch again */
if (node_require) {
	(Module.prototype as any).require = monkey_patch_require;
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
	lib: ['lib.es2017.d.ts'],
};

/** Generate diagnostics if any, the return value true means there was an error, false otherwise */
const generateDiagnostics = (program: ts.Program, diagnostics: readonly ts.Diagnostic[], errors: readonly ts.Diagnostic[]) => {
	const allDiagnostics = ts.getPreEmitDiagnostics(program).concat(diagnostics, errors);

	if (allDiagnostics.length) {
		const formatHost: ts.FormatDiagnosticsHost = {
			getCanonicalFileName: (path) => path,
			getCurrentDirectory: ts.sys.getCurrentDirectory,
			getNewLine: () => ts.sys.newLine,
		};
		const message = ts.formatDiagnosticsWithColorAndContext(allDiagnostics, formatHost);
		console.log(message);
	}
};

const getProgramOptions = (paths: string[] = []) => {
	const defaultOptions = { options: defaultCompilerOptions, rootNames: paths, configFileParsingDiagnostics: [] };
	const configFile = ts.findConfigFile(
		process.cwd(),
		ts.sys.fileExists,
		'tsconfig.json',
	);
	if (!configFile) {
		return defaultOptions;
	}
	const { config, error } = ts.readConfigFile(configFile, ts.sys.readFile);
	if (error) {
		return { ...defaultOptions, configFileParsingDiagnostics: [error] };
	}
	const { options, fileNames, errors } = ts.parseJsonConfigFileContent(
		config,
		ts.sys,
		process.cwd(),
	);
	return { options: errors.length > 0 ? defaultCompilerOptions : options, rootNames: fileNames || paths, configFileParsingDiagnostics: errors };
};

const getTranspileOptions = (moduleName: string, path: string) => {
	const programOptions = getProgramOptions([path]);
	return {
		programOptions,
		transpileOptions: {
			compilerOptions: programOptions.options,
			fileName: path,
			reportDiagnostics: true,
			moduleName,
		},
	};
};

const getMetacallExportTypes = (
	p: ts.Program,
	paths: string[] = [],
	cb: (sourceFile: ts.SourceFile, metacallType: MetacallExports) => void =
		() => { },
) => {
	const exportTypes: MetacallExports = {};
	const files = paths.length === 0 ?
		p.getRootFileNames() :
		paths.map(fileResolveNoThrow).filter(file => p.getRootFileNames().map(path.normalize).includes(path.normalize(file)));
	const sourceFiles = files.map((name) =>
		[name, p.getSourceFile(name)] as const
	);
	for (const [fileName, sourceFile] of sourceFiles) {
		if (!sourceFile) {
			// TODO: Implement better exception handling
			console.log(`Error: Failed to load ${fileName}`);

			const emitResult = p.emit();
			generateDiagnostics(p, emitResult.diagnostics, []);

			return null;
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

const fileResolve = (p: string): string => {
    try {
        return node_resolve(p);
    } catch (ex) {
        if (ex.code !== 'MODULE_NOT_FOUND') {
            throw ex;
        }

        // Try global paths
        const paths = (process.env['NODE_PATH'] || '').split(path.delimiter).filter((e) => e.length !== 0);

        for (const r of paths) {
            try {
                return node_resolve(path.join(r, p));
            } catch (e) {
                if (e.code !== 'MODULE_NOT_FOUND') {
                    throw e;
                }
            }
        }
    }

    throw Object.assign(Error(`Cannot find module '${p}'`), { code: 'MODULE_NOT_FOUND' });
};

const fileResolveNoThrow = (p: string): string => {
	try {
		return fileResolve(p);
	} catch (_) {
		return p;
	}
};

/** Loads a TypeScript file from disk */
export const load_from_file = safe(function load_from_file(paths: string[], discover = true) {
	const result: MetacallHandle = {};
	const options = getProgramOptions(paths.map(p => fileResolve(p)));
	const p = ts.createProgram(options);
	// TODO: Handle the emitSkipped?
	const exportTypes = getMetacallExportTypes(p, paths, (sourceFile, exportTypes) => {
		const { diagnostics /*, emitSkipped */ } = p.emit(sourceFile, (fileName, data) => {
			// @ts-ignore
			const nodeModulePaths = Module._nodeModulePaths(path.dirname(fileName));
			const parent = module.parent;
			const m = new Module(fileName, parent || undefined);
			m.filename = fileName;
			m.paths = nodeModulePaths;
			(m as any)._compile(data, fileName);
			const wrappedExports = wrapFunctionExport(m.exports);
			for (const [name, handle] of Object.entries(exportTypes)) {
				handle.ptr = wrappedExports[name] as anyF;
			}
			if (discover) {
				discoverTypes.set(fileName, {
					...(discoverTypes.get(fileName) ?? {}),
					...exportTypes,
				});
			}
			result[fileName] = wrappedExports;
		});

		generateDiagnostics(p, diagnostics, options.configFileParsingDiagnostics);
	});

	return exportTypes === null ? null : result;
}, null);

/** Loads a TypeScript file from memory */
export const load_from_memory = safe(
	function load_from_memory(name: string, data: string) {
		const extName = `${name}.ts`;
		const { programOptions, transpileOptions } = getTranspileOptions(name, extName);
		const transpileOutput = ts.transpileModule(data, transpileOptions);
		const target = programOptions.options.target ?? defaultCompilerOptions.target;
		const p = ts.createProgram([extName], programOptions.options, {
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
						const tsPath = path.join(path.dirname(node_resolve('typescript')), fileName);
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
		if (exportTypes === null) {
			// TODO: Improve error handling
			return null;
		}
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

/** Registers the TypeScript loader in the correct thread, this loader must always be the child of NodeJS loader */
export const initialize = safe(function initialize(register_cb: (impl: unknown) => boolean, impl: unknown): boolean {
	return register_cb(impl);
}, false);

/** Unloads a TypeScript file using handle returned from load_from_file / load_from_memory */
export const clear = safe(function clear(handle: Record<string, string>) {
	const names = Object.getOwnPropertyNames(handle);

	for (let i = 0; i < names.length; ++i) {
		const p = names[i];

		discoverTypes.delete(p);

		try {
			const absolute = fileResolve(p);

			if (node_cache[absolute]) {
				delete node_cache[absolute];
			}
		} catch (_) {}
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

/** Destroy TypeScript loader children in the correct thread, this loader must always be the child of NodeJS loader */
export const destroy = safe(function destroy(unload_children_cb: (impl: unknown) => boolean, impl: unknown): boolean {
	return unload_children_cb(impl);
}, false);

/** Unimplemented */
export const load_from_package = noop;
