#!/usr/bin/env node

const jwt = require('jsonwebtoken');

const secret = 'secret';

function sign(data) {
	return jwt.sign(data, secret);
}

function verify(token) {
	return jwt.verify(token, secret);
}

module.exports = {
	sign,
	verify,
};
