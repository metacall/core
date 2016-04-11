#!/bin/bash

INCLUDE_PATH=/usr/local/include/mozjs-48a1/
LIB_PATH=/usr/local/lib/

# debug
#g++ -g -I../../../v8/include shell.cc -o example_debug.exe -Wl,--start-group ../../../v8/out/x64.debug/obj.target/{tools/gyp/libv8_{base,libbase,external_snapshot,libplatform},third_party/icu/libicu{uc,i18n,data}}.a -Wl,--end-group -lrt -ldl -pthread -std=c++0x

# release
g++ -I$INCLUDE_PATH -L$LIB_PATH main.cpp -o main.exe -lmozjs-48a1 -std=c++0x

exit 0
