#!/bin/bash

swig3.0 -c++ -javascript -v8 -DV8_VERSION=0x0409385 myextension.i

#gcc myextension_wrap.c -o myextension_wrap.o -std=gnu99 -Wall

exit 0
