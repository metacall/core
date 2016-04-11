#!/bin/bash

# for solving ruby/config.h bug, run:
#	sudo ln -s /usr/include/x86_64-linux-gnu/ruby-2.1.0/ruby/config.h \
#		   /usr/include/ruby-2.1.0/ruby/config.h

INCLUDE_PATH=/usr/include/ruby-2.1.0/

gcc -I$INCLUDE_PATH main.c -o main.exe -std=gnu99 -Wall -lruby-2.1

exit 0
