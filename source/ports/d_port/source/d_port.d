module metacall;

import std.exception;
import core.vararg;

extern(C) {
  int _metacall_initialize();
  int _metacall_destroy();
  int _metacall_load_from_file(const(char*) tag, const(char**) paths, size_t size, void** handle);
  void* metacall_value_create_bool(bool b);
  void* metacall_value_create_char(char c);
  void* metacall_value_create_short(short s);
  void* metacall_value_create_int(int i);
  void* metacall_value_create_long(long l);
  void* metacall_value_create_float(float f);
  void* metacall_value_create_double(double d);
  void* metacall_value_create_string(const(char*) str, size_t length);
  void* metacall_value_create_buffer(const(void*) buffer, size_t size);
  void* metacall_value_create_array(const(void**) values, size_t size);
  void* metacall_value_create_ptr(const(void*) ptr);
  void* metacall_value_create_null();
  void* _metacall(const(char*) name, ...);
  void* metacallv(const(char*) name, void** args);
}

class MetaCallException : Exception {
  mixin basicExceptionCtors;
}

int metacall_initialize() {
  if(_metacall_initialize() != 0) {
    throw new MetaCallException("failed to initialize MetaCall");
  }
  return 0;
}

int metacall_load_from_file(const char[] tag, char[][] paths) {
  int size = paths.sizeof;
  if(_metacall_load_from_file(tag.ptr,cast(const(char**))paths.ptr,size,null) != 0) {
    throw new MetaCallException("MetaCall failed to load script(s)");
  }
  return 0;
}

void* metacall(T...)(char[] name,T args) {
  void*[args.length] values;
  int i = 0;
  foreach(arg; args) {
    if(is(arg == bool)) {
      values[i] = metacall_value_create_bool(cast(int)arg);
    } else if(is(arg == char)) {
      values[i] = metacall_value_create_char(arg);
    } else if(is(arg == short)) {
      values[i] = metacall_value_create_short(arg);
    } else if(is(arg == int)) {
      values[i] = metacall_value_create_int(arg);
    } else if(is(arg == long)) {
      values[i] = metacall_value_create_long(arg);
    } else if(is(arg == float)) {
      values[i] = metacall_create_value_float(arg);
    } else if(is(arg == double)) {
      values[i] = metacall_create_value_double(arg);
    } else if(is(arg == string) || is(arg == char[])) {
      values[i] = metacall_create_value_string(arg.ptr,arg.sizeof);
    } else if(is(arg == ubyte[]) || is(arg == char[]) || is(arg == char*)) {
      values[i] = metacall_create_value_buffer(cast(const(void*))arg.ptr,arg.sizeof);
    } else if(is(arg == void*)) {
      values[i] = metacall_create_value_ptr(arg);
    }
    i += 1;
  }
  return _metacallv(cast(const char*)name.ptr,cast(void**)values.ptr);
}

int metacall_destroy() {
  return _metacall_destroy();
}
