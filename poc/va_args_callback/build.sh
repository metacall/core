#!/bin/bash

gcc -I. main.c -o main_call.exe -Wall
gcc -I. -DFUNC_VA_ARGS -DFUNC_VA_ARGS_1 main.c -o main_va_args_1.exe -Wall
gcc -I. -DFUNC_VA_ARGS -DFUNC_VA_ARGS_2 main.c -o main_va_args_2.exe -Wall
gcc -I. -DFUNC_VA_ARGS -DFUNC_VA_ARGS_3 main.c -o main_va_args_3.exe -Wall
gcc -I. -DFUNC_VA_ARGS -DFUNC_VA_ARGS_4 main.c -o main_va_args_4.exe -Wall

exit 0
