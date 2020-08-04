import std.stdio;
import std.variant;
import metacall;

int main()
{
	writeln("intializing metacall");
	dmetacall_initialize();
	writeln("loading script");
	string[] scripts = [
		"script.py"
	];
	dmetacall_load_from_file("py",scripts);
	Variant ret = dmetacall!(int,int)("add",1,2);
	writefln("add(1,2) = %d",ret.get!(long));
	writeln("destroying metacall");
	dmetacall_destroy();
	return 0;
}
