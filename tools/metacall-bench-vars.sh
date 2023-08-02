#!/bin/bash

# Extract the OS name from matrix.os
OS_NAME=$(echo $1 | sed 's/-latest$//')
# Extract the benchmark name from matrix.benchs
BENCH_NAME=$(echo $2 | sed 's/metacall-\(.*\)-bench.json/\1/')
# Set the OS_NAME environment variable for use in subsequent steps
echo "OS_NAME=${OS_NAME}" >> $GITHUB_ENV
echo "BENCH_NAME=${BENCH_NAME}" >> $GITHUB_ENV
echo "BENCH_PATH=./build/" >> $GITHUB_ENV
