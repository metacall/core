#!/bin/bash

gcc -I. function.c main.c -o main.exe -std=gnu99 -Wall -lffi

exit 0
