long c_long_callback(long (*sum)(long, long))
{
	return sum(3L, 4L);
}

void c_void_callback(void (*vd)(void))
{
	vd();
}

char *c_str_callback(char *(*cb)(void))
{
	return cb();
}
