const { spawn } = require('child_process');
const http = require('http');

// Start mock server
const server = spawn(process.argv[0], [process.argv[2]]);

server.stdout.pipe(process.stdout);
server.stderr.pipe(process.stderr);

server.on('exit', (code) => {
	if (code !== 0) {
		process.exit(code);
	}
});

// Check if server is ready
function isReady() {
	return new Promise((resolve, reject) => {
		const options = {
			host: 'localhost',
			port: 6094,
			path: '/ready',
		};

		const callback = (res) => {
			let data = '';
		
			res.on('data', (chunk) => {
				data += chunk;
			});
			
			res.on('end', () => {
				resolve(data === 'OK');
			});

			res.on('error', reject);
		};
		try {
			const req = http.request(options, callback);
			req.on('error', reject);
			req.end();
		} catch (e) {
			reject(e);
		}
	});
}

// Catch unhandled exceptions
function killTest(error) {
	server.kill('SIGINT');
	console.error(error);
	process.exit(1);
}

process.on('uncaughtException', killTest);

// Wait server to be ready and execute the test
(async function run() {
	let ready = false;

	setTimeout(() => {
		if (ready === false) {
			killTest('Timeout reached, server is not ready');
		}
	}, 10000);

	while (ready !== true) {
		try {
			ready = await isReady();
		} catch (e) { }
	}

	console.log('Starting the test');

	const test = spawn(process.argv[3]);

	test.stdout.pipe(process.stdout);
	test.stderr.pipe(process.stderr);
	
	test.on('exit', (code) => {
		if (code !== 0) {
			killTest(`Error: Test exited with code ${code}`);
		}
		process.exit(0);
	});
})();
