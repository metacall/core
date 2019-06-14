const addon = require("./build/Debug/addon.node");
addon.metacall_load_from_file("mock", ["asd.mock"]);
var b = addon.metacall("my_empty_func")
console.log(b)
