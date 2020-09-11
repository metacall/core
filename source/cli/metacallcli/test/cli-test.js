// Require MetaCall directly from Node Port
require('../../source/ports/node_port/index.js');

const { a } = require('cli-test-target.py');

console.log(a());
