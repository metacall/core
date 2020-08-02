module metacall;

import std.stdio;
import std.exception;
import std.string;
import std.variant;
import core.vararg;
import core.stdc.string;

enum metacall_value_id_enum : int {
  METACALL_BOOL		  = 0,
  METACALL_CHAR		  = 1,
  METACALL_SHORT	  = 2,
  METACALL_INT		  = 3,
  METACALL_LONG		  = 4,
  METACALL_FLOAT	  = 5,
  METACALL_DOUBLE	  = 6,
  METACALL_STRING	  = 7,
  METACALL_BUFFER	  = 8,
  METACALL_ARRAY	  = 9,
  METACALL_MAP		  = 10,
  METACALL_PTR		  = 11,
  METACALL_FUTURE	  = 12,
  METACALL_FUNCTION	= 13,
  METACALL_NULL		  = 14,
  METACALL_SIZE,
  METACALL_INVALID
}

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
  char* metacall_value_to_string(void* v);
  void metacall_value_destroy(void*);
  metacall_value_id_enum metacall_value_id(void* v);
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

Variant dmetacall(T...)(string name,T args) {
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

  void* ret = metacallv(name.toStringz(),cast(void**)values.ptr);
  foreach(void* value; values) {
    metacall_value_destroy(value);
  }

  switch(metacall_value_id(ret)) {
    case metacall_value_id_enum.METACALL_CHAR:
      return Variant(metacall_value_to_char(ret));
    case metacall_value_id_enum.METACALL_SHORT:
      return Variant(metacall_value_to_short(ret));
    case metacall_value_id_enum.METACALL_INT:
      return Variant(metacall_value_to_int(ret));
    case metacall_value_id_enum.METACALL_LONG:
      return Variant(metacall_value_to_long(ret));
    case metacall_value_id_enum.METACALL_FLOAT:
      return Variant(metacall_value_to_float(ret));
    case metacall_value_id_enum.METACALL_DOUBLE:
      return Variant(metacall_value_to_double(ret));
    case metacall_value_id_enum.METACALL_STRING:
      return Variant(metacall_value_to_string(ret).fromStringz());
    case metacall_value_id_enum.METACALL_PTR:
      return Variant(metacall_value_to_ptr(ret));
    case metacall_value_id_enum.METACALL_NULL:
      return Variant(null);
    default:
      return Variant(null);
  }
}

int dmetacall_destroy() {
  return metacall_destroy();
}
