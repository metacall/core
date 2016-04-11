#include <ruby.h>

VALUE say_hello_stub(VALUE args)
{
	rb_funcall(rb_mKernel, rb_intern("say_hello"), 1, rb_ary_entry(args, 0));

	return Qundef;
}

void say_hello_call(char *name)
{
	int state;

	VALUE name_value = rb_str_new2(name);

	rb_protect(say_hello_stub, rb_ary_new_from_args(1, name_value), &state);

	if (state)
	{
		printf("Error when calling function\n");
	}
}

int main(int argc, char *argv[])
{
	VALUE script;

	int state;

	ruby_init();

	ruby_init_loadpath();

	script = rb_str_new2("./hello_world.rb");

	rb_load(script, 0);

	rb_load_protect(script, 0, &state);

	if (state == 0)
	{
		printf("Script loaded successfully\n");

		say_hello_call("monkey");
	}

	ruby_cleanup(0);

	return 0;
}
