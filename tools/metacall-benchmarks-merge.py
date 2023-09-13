#!/usr/bin/python3
import json
import os
import sys

# Validate arguments
if len(sys.argv) != 2:
	print('Invalid number of arguments, you should pass the location of the benchmarks.')
	print('Usage: python3 metacall-benchmarks-merge.py ${CMAKE_BINARY_DIR}/benchmarks')
	exit(1)

# Validate the benchmark path
if not os.path.isdir(sys.argv[1]):
	print('The directory \'' + sys.argv[1] + '\' does not exist or is not a valid path.')
	exit(2)

# Search all benchmarks and merge them
output = {}

for file in os.listdir(sys.argv[1]):
	if file.endswith('.json'):
		f = open(os.path.join(sys.argv[1], file), 'r')

		# Sanitize the data (https://github.com/google/benchmark/issues/784)
		# '-Infinity', 'Infinity', 'NaN'
		def sanitize(arg):
			c = {
				'-Infinity': sys.float_info.min, # -float('inf')
				'Infinity': sys.float_info.max, # float('inf')
				'NaN': 0 # float('nan')
			}
			# TODO: Eventually solve this from the root of problem in Windows
			print('Warning: Got value "' + arg + '" in the test ' + file + ', review why it is failing')
			return c[arg]

		data = json.loads(f.read(), parse_constant=sanitize)

		if not output:
			output = data
		else:
			for benchmark in data['benchmarks']:
				output['benchmarks'].append(benchmark)

		f.close()

# Check if we found data
if not output:
	print('The directory \'' + sys.argv[1] + '\' does not contain any benchmark.')
	exit(3)

# Rename the binary file
output['context']['executable'] = 'benchmarks'

# Store the result
dest = os.path.join(sys.argv[1], 'metacall-benchmarks.json')
f = open(dest, 'w+')
json.dump(output, f, indent = 4)
f.close()
