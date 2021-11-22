long c_long_callback(long (*sum)(long, long))
{
	return sum(3L, 4L);
}

void c_void_callback(void (*vd)())
{
	vd();
}

char *c_str_callback(char *(*cb)())
{
	return cb();
}
