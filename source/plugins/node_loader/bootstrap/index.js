#!/usr/bin/env node

const binding = require('binding');

module.exports = binding.register(process.argv[2]);
