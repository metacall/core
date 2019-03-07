#!/usr/bin/env node

function sign(data) {
	return data;
}

function verify(token) {
	return true;
}

module.exports = {
	sign,
	verify,
};
