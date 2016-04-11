#!/bin/bash

# for solving ruby/config.h bug, run:
#	sudo ln -s /usr/include/x86_64-linux-gnu/ruby-2.1.0/ruby/config.h \
#		   /usr/include/ruby-2.1.0/ruby/config.h

RUBY_INCLUDE_PATH=/usr/include/ruby-2.1.0/

gcc -I$RUBY_INCLUDE_PATH -I. `python3.4-config --cflags` function.c ruby_backend.c python_backend.c main.c -o main.exe -std=gnu99 -Wall `python3.4-config --ldflags` -lruby-2.1 -lffi

exit 0

