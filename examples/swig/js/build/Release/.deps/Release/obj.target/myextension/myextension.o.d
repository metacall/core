cmd_Release/obj.target/myextension/myextension.o := g++ '-DNODE_GYP_MODULE_NAME=myextension' '-D_LARGEFILE_SOURCE' '-D_FILE_OFFSET_BITS=64' '-DBUILDING_NODE_EXTENSION' -I/home/monkey/.node-gyp/5.7.1/include/node -I/home/monkey/.node-gyp/5.7.1/src -I/home/monkey/.node-gyp/5.7.1/deps/uv/include -I/home/monkey/.node-gyp/5.7.1/deps/v8/include  -fPIC -pthread -Wall -Wextra -Wno-unused-parameter -m64 -O3 -ffunction-sections -fdata-sections -fno-omit-frame-pointer -fno-rtti -fno-exceptions -std=gnu++0x -MMD -MF ./Release/.deps/Release/obj.target/myextension/myextension.o.d.raw   -c -o Release/obj.target/myextension/myextension.o ../myextension.cxx
Release/obj.target/myextension/myextension.o: ../myextension.cxx \
 ../myextension.h
../myextension.cxx:
../myextension.h:
