const path = require("path");
switch(process.platform){
	case "win32" :
		break;
	case "freebsd":
		break;
	case "linux":
		var dir = path.resolve(__dirname, "lib");
		console.log("-L"+dir)
		console.log("-lmetacall")
		console.log('-Wl,-rpath,' + 'lib')
		break;
	case "darwin":
		break;
	default:
		throw Error("This Node Package is not supported in your platform");
}