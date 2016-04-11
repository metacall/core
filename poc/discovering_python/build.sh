#!/bin/bash

# for solving ruby/config.h bug, run:
#	sudo ln -s /usr/include/x86_64-linux-gnu/ruby-2.1.0/ruby/config.h \
#		   /usr/include/ruby-2.1.0/ruby/config.h

RUBY_INCLUDE_PATH=/usr/include/ruby-2.1.0/

gcc -I. function.c -c -std=gnu99 -Wall -lffi
gcc -I$RUBY_INCLUDE_PATH -I. ruby_backend.c -c -std=gnu99 -lruby-2.1
gcc `python3.4-config --cflags` -I. python_backend.c -c -std=gnu99 `python3.4-config --ldflags`

gcc -I. function.o ruby_backend.o python_backend.o main.c -o main.exe -std=gnu99 -lffi -lruby-2.1 `python3.4-config --ldflags`

exit 0

