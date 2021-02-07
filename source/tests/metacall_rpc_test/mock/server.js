const http = require('http');
const port = 6094;

const server = http.createServer((req, res) => {
	req.on('error', err => {
		console.error(err);
		process.exit(1);
	});

	res.on('error', err => {
		console.error(err);
		process.exit(1);
	});

	const data = new Promise((resolve) => {
		let body = [];

		req.on('data', (chunk) => {
			body.push(chunk);
		}).on('end', () => {
			resolve(Buffer.concat(body).toString());
		});
	});

	if (req.method === 'GET') {
		if (req.url === '/viferga/example/v1/inspect') {
			const inspect = '{"py":[{"name":"example.py","scope":{"name":"global_namespace","funcs":[{"name":"divide","signature":{"ret":{"type":{"name":"float","id":6}},"args":[{"name":"left","type":{"name":"float","id":6}},{"name":"right","type":{"name":"float","id":6}}]},"async":false},{"name":"hello","signature":{"ret":{"type":{"name":"","id":18}},"args":[]},"async":false},{"name":"return_same_array","signature":{"ret":{"type":{"name":"","id":18}},"args":[{"name":"arr","type":{"name":"","id":18}}]},"async":false},{"name":"bytebuff","signature":{"ret":{"type":{"name":"bytes","id":8}},"args":[{"name":"input","type":{"name":"bytes","id":8}}]},"async":false},{"name":"dont_load_this_function","signature":{"ret":{"type":{"name":"","id":18}},"args":[{"name":"left","type":{"name":"","id":18}},{"name":"right","type":{"name":"","id":18}}]},"async":false},{"name":"sum","signature":{"ret":{"type":{"name":"int","id":4}},"args":[{"name":"left","type":{"name":"int","id":4}},{"name":"right","type":{"name":"int","id":4}}]},"async":false},{"name":"strcat","signature":{"ret":{"type":{"name":"str","id":7}},"args":[{"name":"left","type":{"name":"str","id":7}},{"name":"right","type":{"name":"str","id":7}}]},"async":false},{"name":"return_array","signature":{"ret":{"type":{"name":"","id":18}},"args":[]},"async":false},{"name":"multiply","signature":{"ret":{"type":{"name":"int","id":4}},"args":[{"name":"left","type":{"name":"int","id":4}},{"name":"right","type":{"name":"int","id":4}}]},"async":false}],"classes":[],"objects":[]}}],"rb":[{"name":"hello.rb","scope":{"name":"global_namespace","funcs":[{"name":"say_multiply","signature":{"ret":{"type":{"name":"","id":18}},"args":[{"name":"left","type":{"name":"Fixnum","id":3}},{"name":"right","type":{"name":"Fixnum","id":3}}]},"async":false},{"name":"get_second","signature":{"ret":{"type":{"name":"","id":18}},"args":[{"name":"first","type":{"name":"Fixnum","id":3}},{"name":"second","type":{"name":"Fixnum","id":3}}]},"async":false},{"name":"say_null","signature":{"ret":{"type":{"name":"","id":18}},"args":[]},"async":false},{"name":"say_hello","signature":{"ret":{"type":{"name":"","id":18}},"args":[{"name":"value","type":{"name":"String","id":7}}]},"async":false},{"name":"backwardsPrime","signature":{"ret":{"type":{"name":"","id":18}},"args":[{"name":"start","type":{"name":"","id":18}},{"name":"stop","type":{"name":"","id":18}}]},"async":false},{"name":"get_second_untyped","signature":{"ret":{"type":{"name":"","id":18}},"args":[{"name":"first","type":{"name":"","id":18}},{"name":"second","type":{"name":"","id":18}}]},"async":false},{"name":"say_sum_ducktyped","signature":{"ret":{"type":{"name":"","id":18}},"args":[{"name":"left","type":{"name":"","id":18}},{"name":"right","type":{"name":"","id":18}}]},"async":false},{"name":"say_string_without_spaces","signature":{"ret":{"type":{"name":"","id":18}},"args":[{"name":"value","type":{"name":"String","id":7}}]},"async":false},{"name":"say_multiply_ducktyped","signature":{"ret":{"type":{"name":"","id":18}},"args":[{"name":"left","type":{"name":"","id":18}},{"name":"right","type":{"name":"","id":18}}]},"async":false}],"classes":[],"objects":[]}}],"cs":[{"name":"hello.cs","scope":{"name":"global_namespace","funcs":[{"name":"Sum","signature":{"ret":{"type":{"name":"int","id":3}},"args":[{"name":"a","type":{"name":"int","id":3}},{"name":"b","type":{"name":"int","id":3}}]},"async":false},{"name":"Say","signature":{"ret":{"type":{"name":"","id":18}},"args":[{"name":"text","type":{"name":"string","id":7}}]},"async":false},{"name":"Concat","signature":{"ret":{"type":{"name":"string","id":7}},"args":[{"name":"a","type":{"name":"string","id":7}},{"name":"b","type":{"name":"string","id":7}}]},"async":false},{"name":"SayHello","signature":{"ret":{"type":{"name":"","id":18}},"args":[]},"async":false}],"classes":[],"objects":[]}}],"__metacall_host__":[],"mock":[{"name":"empty.mock","scope":{"name":"global_namespace","funcs":[{"name":"three_str","signature":{"ret":{"type":{"name":"String","id":7}},"args":[{"name":"a_str","type":{"name":"String","id":7}},{"name":"b_str","type":{"name":"String","id":7}},{"name":"c_str","type":{"name":"String","id":7}}]},"async":false},{"name":"my_empty_func_str","signature":{"ret":{"type":{"name":"String","id":7}},"args":[]},"async":false},{"name":"my_empty_func_int","signature":{"ret":{"type":{"name":"Integer","id":3}},"args":[]},"async":false},{"name":"new_args","signature":{"ret":{"type":{"name":"String","id":7}},"args":[{"name":"a_str","type":{"name":"String","id":7}}]},"async":false},{"name":"two_str","signature":{"ret":{"type":{"name":"String","id":7}},"args":[{"name":"a_str","type":{"name":"String","id":7}},{"name":"b_str","type":{"name":"String","id":7}}]},"async":false},{"name":"two_doubles","signature":{"ret":{"type":{"name":"Double","id":6}},"args":[{"name":"first_parameter","type":{"name":"Double","id":6}},{"name":"second_parameter","type":{"name":"Double","id":6}}]},"async":false},{"name":"my_empty_func","signature":{"ret":{"type":{"name":"Integer","id":3}},"args":[]},"async":false},{"name":"mixed_args","signature":{"ret":{"type":{"name":"Char","id":1}},"args":[{"name":"a_char","type":{"name":"Char","id":1}},{"name":"b_int","type":{"name":"Integer","id":3}},{"name":"c_long","type":{"name":"Long","id":4}},{"name":"d_double","type":{"name":"Double","id":6}},{"name":"e_ptr","type":{"name":"Ptr","id":11}}]},"async":false}],"classes":[],"objects":[]}}]}';
			res.setHeader('Content-Type', 'text/json');
			res.end(inspect);
			return;
		}
	} else if (req.method === 'POST') {
		if (req.url === '/viferga/example/v1/call/divide') {
			data.then((body) => {
				if (body !== '[50,10]') {
					console.error('Invalid body:', body);
					process.exit(1);
				}
				const result = '5.0';
				res.setHeader('Content-Type', 'text/json');
				res.end(result);
			});
			return;
		}
	}

	console.error('Invalid request method or url:', req.method, req.url);
	process.exit(1);
});

server.listen(port, () => {
	console.log(`MetaCall server listening at ${port}`);
});
