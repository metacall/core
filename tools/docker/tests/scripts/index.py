from metacall import metacall, metacall_load_from_file

assert metacall_load_from_file('c', ['./bench.c'])

print(metacall('bench_xyz789', 1234, 15))
