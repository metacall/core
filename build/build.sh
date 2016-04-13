#!/bin/bash

INCLUDE_PATH=../src/.

# for solving ruby/config.h bug, run:
#	sudo ln -s /usr/include/x86_64-linux-gnu/ruby-2.1.0/ruby/config.h \
#		   /usr/include/ruby-2.1.0/ruby/config.h

RUBY_INCLUDE_PATH=/usr/include/ruby-2.1.0/

# build loader implementations

#gcc -I$INCLUDE_PATH ../src/loader/loader_impl_c.c ../src/loader/loader_c.c -c -std=gnu99 -Wall -lffi
#gcc -I$RUBY_INCLUDE_PATH -I$INCLUDE_PATH ../src/loader/loader_impl_rb.c ../src/loader/loader_rb.c -c -std=gnu99 -lruby-2.1
gcc `python3.4-config --cflags` -I$INCLUDE_PATH ../src/loader/loader_impl_py_discovering.c ../src/loader/loader_impl_py.c ../src/loader/loader_py.c -c -std=gnu99 `python3.4-config --ldflags`

# build loader
gcc -I$INCLUDE_PATH ../src/loader/loader_naming.c ../src/loader/loader_manager.c ../src/loader.c -c -std=gnu99 -lffi -lruby-2.1 `python3.4-config --ldflags`

# build call

#gcc -I$INCLUDE_PATH loader.o ../src/call.c ../ -c -std=gnu99 -lffi -lruby-2.1 `python3.4-config --ldflags`


exit 0

