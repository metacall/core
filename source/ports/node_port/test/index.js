'use strict';

/* TODO: Review why index.js is not working */

// const { metacall, metacall_load_from_file } = require('../index.js');
const { metacall, metacall_load_from_file } = require('../build/node_portd.node');

metacall_load_from_file('mock', ['asd.mock']);

const result = metacall('my_empty_func');

console.log(result);
