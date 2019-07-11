const addon = require('./build/Release/metacall.node');

/* TODO: Override require and monkey patch the functions */

module.exports = {
	metacall() {
		if (arguments.length === 0) {
			throw Error('At least one argument should be passed indicating the name of the function to be called.');
		}

		if (typeof arguments[0] !== 'string') {
			throw Error('Function name should be of string type.');
		}

		addon.metacall(...arguments);
	},

	metacall_load_from_file(tag, paths) {
		if (typeof tag !== 'string') {
			throw Error('Tag should be a string indicating the id of the loader to be used [py, rb, cs, js, node...].');
		}

		if (!(paths instanceof Array)) {
			throw Error('Paths should be an array with file names and paths to be loaded by the loader.');
		}

		addon.metacall_load_from_file(tag, paths);
	},
}
