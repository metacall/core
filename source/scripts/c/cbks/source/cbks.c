long c_long_callback(long (*sum)(long, long))
{
	return sum(3L, 4L);
}

void c_void_callback(void (*vd)())
{
	vd();
}
