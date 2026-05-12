from metacall import metacall, metacall_load_from_file, metacall_load_from_file_export
import platform

# C
assert metacall_load_from_file('c', ['./bench.c'])
result = metacall('bench_xyz789', 1234, 15)
print(result)
assert isinstance(result, float) and result != 0.0, f"Expected non-zero float, but got {type(result)} with value {result}"

# Rust
if platform.machine().lower() not in ("x86_64", "amd64") or platform.architecture()[0] != '64bit':
	print(f"TODO: Rust not working for {platform.machine()} architecture.")
else:
	assert metacall_load_from_file('rs', ['./bench.rs'])
	result = metacall('pairwise_suffix_sum', [4.4, 5.5, 6.6, 7.7])
	print(result)
	assert isinstance(result, float) and result == 369.04999999999995

# Java
# greeting = metacall_load_from_file_export('java', ['./GreetingService.java'])
# print(greeting)

# import GreetingService.java

# greeting = GreetingService()
# result = greeting.getGreeting()
# assert isinstance(result, str) and result == 'Hello from Java!'
