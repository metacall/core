#!/usr/bin/env node

const http = require('http');

const server = http.createServer((req, res) => {
	res.writeHead(200);
	res.end('Hello, World!');
});

const print = (text) => {
	console.log('###################################');
	console.log(text);
	console.log('###################################');
}

server.listen(8080, () => print('Server up'));

setTimeout(() => {
	print('Closing server...');
	server.close();
	print('Server closed');
}, 5000);
