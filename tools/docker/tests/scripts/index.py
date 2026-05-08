from metacall import metacall, metacall_load_from_file

# C
assert metacall_load_from_file('c', ['./bench.c'])

result = metacall('bench_xyz789', 1234, 15)

print(result)

assert isinstance(result, float) and result != 0.0, f"Expected non-zero float, but got {type(result)} with value {result}"

# Rust
assert metacall_load_from_file('rs', ['./bench.rs'])

result = metacall('pairwise_suffix_sum', [4.4, 5.5, 6.6, 7.7])

print(result)

assert isinstance(result, float) and result == 0.0, f"Expected non-zero float, but got {type(result)} with value {result}"
