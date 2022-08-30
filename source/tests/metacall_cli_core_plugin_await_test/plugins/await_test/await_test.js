const { metacall_load_from_file } = require(process.env['METACALL_NODE_PORT_PATH']);

function await__test(await_cb) {
	metacall_load_from_file("node", ["scripts/nod.js"]);
	hello_await = 'hello_boy_await(1,2)';
	console.log(await_cb);

	// TODO: This call generates a deadlock
	await_cb(hello_await);

	return 22;
}

module.exports = await__test
