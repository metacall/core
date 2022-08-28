const { metacall, metacall_load_from_file, metacall_inspect } = require('metacall')

function await__test(await_cb) {
	metacall_load_from_file("node", ["scripts/nod.js"]);
	hello_await = 'hello_boy_await(1,2)';
	await_cb(hello_await);

	return 22;
}

module.exports = await__test
