#!/usr/bin/env node

'use strict';

const path = require('path');

/* Load MetaCall addon */
const addon = (() => {
	try {
		/* This forces metacall port to be run always by metacall cli */
		return process._linkedBinding('node_loader_port_module');
	} catch (e) {
		console.error('MetaCall failed to load, probably you are importing this file from NodeJS directly.');
		console.error('You should use MetaCall CLI instead. Install it from: https://github.com/metacall/install');
		throw e;
	}
})();

const script = `#!/usr/bin/env python3

import os
import sys

sys.path.append(os.environ['PY_PORT_LIBRARY_PATH']);

from metacall import metacall

def b():
	return metacall('c');
`;

addon.metacall_load_from_memory('py', script);

function a() {
	console.log('-------------------------------------------');
	console.log(addon.metacall_inspect());
	console.log('-------------------------------------------');
	return addon.metacall('b');
}

function c() {
	return 3.0;
}

module.exports = {
	a,
	c,
};
