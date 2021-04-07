const path = require('path');
const fs = require('fs');
const ts = require('typescript');
const { config, error } = ts.readConfigFile(path.join(path.resolve(__dirname), 'tsconfig.json'), ts.sys.readFile);
if (error) {
	console.error(error);
	process.exit(1);
}
const { options, fileNames, errors } = ts.parseJsonConfigFileContent(
	config,
	ts.sys,
	path.resolve(__dirname),
);
if (errors.length > 0) {
	console.error(errors);
	process.exit(1);
}
fileNames.map(f => {
	const data = fs.readFileSync(f, { encoding: 'utf8', flag: 'r' });
	const result = ts.transpileModule(data, {
		compilerOptions: options,
		reportDiagnostics: false,
	});
	const outPath = path.join(options.outDir, `${path.parse(f).name}.js`);
	fs.writeFileSync(outPath, result.outputText, { encoding: 'utf-8' });
});
