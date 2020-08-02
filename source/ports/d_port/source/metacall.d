module metacall;

import std.stdio;
import std.exception;
import std.string;
import core.vararg;
import core.stdc.string;

extern(C) {
  int metacall_initialize();
  int metacall_destroy();
  int metacall_load_from_file(const(char*) tag, const(char**) paths, size_t size, void** handle);
  void* metacall_value_create_bool(int b);
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
  char metacall_value_to_char(void* v);
  short metacall_value_to_short(void* v);
  int metacall_value_to_int(void* v);
  long metacall_value_to_long(void* v);
  float metacall_value_to_float(void* v);
  double metacall_value_to_double(void* v);
  void* metacall_value_to_ptr(void* v);
  void metacall_value_destroy(void*);
  void* metacall(const(char*) name, ...);
  void* metacallv(const(char*) name, void** args);
}

class MetaCallException : Exception {
  mixin basicExceptionCtors;
}

int dmetacall_initialize() {
  if(metacall_initialize() != 0) {
    throw new MetaCallException("failed to initialize MetaCall");
  }
  return 0;
}

int dmetacall_load_from_file(const char[] tag, string[] paths) {
  char*[] good_paths;
  good_paths.length += paths.length;
  for(int i = 0; i < paths.length; i++) {
    good_paths[i] = cast(char*)paths[i].toStringz();
  }
  if(metacall_load_from_file(tag.ptr,cast(const(char**))good_paths,paths.length,null) != 0) {
    throw new MetaCallException("MetaCall failed to load script(s)");
  }
  return 0;
}

void* dmetacall(T...)(string name,T args) {
  void*[args.length] values;
  int i = 0;
  foreach(arg; args) {
    static if(is(typeof(arg) == bool)) {
      values[i] = metacall_value_create_bool(cast(int)arg);
    } else static if(is(typeof(arg) == char)) {
      values[i] = metacall_value_create_char(arg);
    } else static if(is(typeof(arg) == short)) {
      values[i] = metacall_value_create_short(arg);
    } else static if(is(typeof(arg) == int)) {
      values[i] = metacall_value_create_int(arg);
    } else static if(is(typeof(arg) == long)) {
      values[i] = metacall_value_create_long(arg);
    } else static if(is(typeof(arg) == float)) {
      values[i] = metacall_value_create_float(arg);
    } else static if(is(typeof(arg) == double)) {
      values[i] = metacall_value_create_double(arg);
    } else static if(is(typeof(arg) == string) || is(typeof(arg) == char[])) {
      values[i] = metacall_value_create_string(arg.ptr,arg.sizeof);
    /*} else static if(is(typeof(arg) == ubyte[]) || is(typeof(arg) == char[]) || is(typeof(arg) == char*)) {
      values[i] = metacall_value_create_buffer(cast(const(void*))arg.ptr,arg.sizeof);*/
    } else static if(is(typeof(arg) == void*)) {
      values[i] = metacall_value_create_ptr(arg);
    }
    i++;
  }
  auto ret = metacallv(name.toStringz(),cast(void**)values.ptr);
  foreach(void* value; values) {
    metacall_value_destroy(value);
  }
  return ret;
}

int dmetacall_destroy() {
  return metacall_destroy();
}
