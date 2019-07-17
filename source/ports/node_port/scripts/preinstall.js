'use strict';

const path = require('path');
const fs = require("fs");

/* TODO */

switch (process.platform)
{
	case "linux":
		if (!fs.existsSync("/usr/lib/libmetacall.so"))
		{
			console.log("I think you haven't installed metacall, coz i can't find it in /usr/lib/")
		}
		break;
	case "win32":
		if (!process.env.METACALL_HOME) console.log("try install metacall with the windows installer")
}
