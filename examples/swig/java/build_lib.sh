#!/bin/bash

# C++ extension
#swig3.0 -c++ -java myextension.i

#g++ myextension.cxx myextension_wrap.cxx -c -I/usr/lib/jvm/java-7-openjdk-amd64/include/ -I/usr/lib/jvm/java-1.7.0-openjdk-amd64/include/

#ld -G myextension.o myextension_wrap.o -o libmyextension.so


# C extension
swig3.0 -java myextension.i

gcc myextension.c myextension_wrap.c -c -fPIC -I/usr/lib/jvm/java-7-openjdk-amd64/include/ -I/usr/lib/jvm/java-1.7.0-openjdk-amd64/include/

ld -G myextension.o myextension_wrap.o -o libmyextension.so

exit 0
