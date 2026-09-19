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

# C#
architecture = platform.machine()
if architecture in ("riscv64", "i386", "i686", "armv7l", "armv6l") or (architecture == "x86_64" and platform.architecture()[0] == "32bit"):
	print(f"netcore8 has no support for {architecture}")
else:
	assert metacall_load_from_file('cs', ['./Sum.cs'])

	result = metacall('sum_cs', 3, 4)
	print(result)
	assert result == 7, f"Expected 7, but got {result}"

	greeting = metacall('greet_cs', 'MetaCall')
	print(greeting)
	assert greeting == 'Hello MetaCall from C#!', f"Unexpected greeting: {greeting}"

# Java
# greeting = metacall_load_from_file_export('java', ['./GreetingService.java'])
# print(greeting)

# import GreetingService.java

# greeting = GreetingService()
# result = greeting.getGreeting()
# assert isinstance(result, str) and result == 'Hello from Java!'
