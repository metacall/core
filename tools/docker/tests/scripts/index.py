from metacall import metacall, metacall_load_from_file

assert metacall_load_from_file('c', ['./bench.c'])

result = metacall('bench_xyz789', 1234, 15)

print(result)

assert isinstance(result, float) and result != 0.0, f"Expected non-zero float, but got {type(result)} with value {result}"
