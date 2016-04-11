#!/bin/bash

# debug
#g++ -g -I../../../../v8/include shell.cc -o example_debug.exe -Wl,--start-group ../../../../v8/out/x64.debug/obj.target/{tools/gyp/libv8_{base,libbase,external_snapshot,libplatform},third_party/icu/libicu{uc,i18n,data}}.a -Wl,--end-group -lrt -ldl -pthread -std=c++0x


# release
#g++ -I../../../../v8/ shell.cc -o example.exe -Wl,--start-group ../../../../v8/out/x64.release/obj.target/{tools/gyp/libv8_{base,libbase,external_snapshot,libplatform},third_party/icu/libicu{uc,i18n,data}}.a -Wl,--end-group -lrt -ldl -pthread -std=c++0x
#g++ -I../../../../v8/ hello-world.cc -o hello-world.exe -Wl,--start-group ../../../../v8/out/x64.release/obj.target/{tools/gyp/libv8_{base,libbase,external_snapshot,libplatform},third_party/icu/libicu{uc,i18n,data}}.a -Wl,--end-group -lrt -ldl -pthread -std=c++0x
g++ -I../../../../v8/ hello-world.cc -o hello-world.exe -Wl,--start-group ../../../../v8/out/native/obj.target/{tools/gyp/libv8_{base,libbase,external_snapshot,libplatform},third_party/icu/libicu{uc,i18n,data}}.a -Wl,--end-group -lrt -ldl -pthread -std=c++0x

exit 0
