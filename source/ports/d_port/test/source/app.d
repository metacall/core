import std.stdio;
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
	void* ret = dmetacall!(int,int)("add",1,2);
	writefln("add(1,2) = %d",metacall_value_to_int(ret));
	writeln("destroying metacall");
	dmetacall_destroy();
	return 0;
}
