package metacall;

import metacall.util.*;

import javax.lang.model.type.NullType;

import com.sun.jna.*;
import com.sun.jna.ptr.PointerByReference;


public interface Bindings extends Library
{

  int metacall_initialize();

  void metacall_log_null();

  int metacall_load_from_file(String tag, String paths[], SIZET size, PointerByReference handle);

  Pointer metacall_handle_export(Pointer handle);

  Pointer metacallv_s(String name, Pointer args[], SIZET size);
  Pointer metacallfv_s(Pointer func, Pointer args[], SIZET size);

  Pointer metacall_await_s(String name, Pointer[] args, SIZET size, ResolveCallback resolve, RejectCallback reject, Pointer data);
  Pointer metacall_await_future(Pointer future, ResolveCallback resolve, RejectCallback reject, Pointer data);

  Pointer metacallhv_s(Pointer handle, String name, Pointer args[], SIZET size);

  int metacall_register(String name, FunctionPointer invoke, PointerByReference func, int ret, SIZET size, int[] types);

  Pointer metacall_function(String name);
  SIZET metacall_function_size(Pointer func);
  int metacall_function_async(Pointer func);

  int metacall_destroy();

  //metacall_value.h
  Pointer metacall_value_create_int(int i);
  Pointer metacall_value_create_long(long l);
  Pointer metacall_value_create_short(short s);
  Pointer metacall_value_create_string(String str, SIZET length);
  Pointer metacall_value_create_char(char c);
  Pointer metacall_value_create_double(double d);
  Pointer metacall_value_create_float(float f);
  Pointer metacall_value_create_boolean(boolean b);
  Pointer metacall_value_create_function(Pointer f);
  Pointer metacall_value_create_array(Pointer values[], SIZET size);
  Pointer metacall_value_create_map(Pointer tuples[], SIZET size);
  Pointer metacall_value_create_null();

  int metacall_value_to_int(Pointer v);
  long metacall_value_to_long(Pointer v);
  short metacall_value_to_short(Pointer v);
  float metacall_value_to_float(Pointer v);
  double metacall_value_to_double(Pointer v);
  boolean metacall_value_to_bool(Pointer v);
  char metacall_value_to_char(Pointer v);
  String metacall_value_to_string(Pointer v);
  Pointer[] metacall_value_to_array(Pointer v);
  NullType metacall_value_to_null(Pointer v);

  Pointer metacall_value_to_function(Pointer v);
  Pointer[] metacall_value_to_map(Pointer v);
  Pointer metacall_value_to_future(Pointer v);

  Pointer metacall_value_from_int(Pointer v, int i);
  Pointer metacall_value_from_string(Pointer v, String str, SIZET length);

  SIZET metacall_value_size(Pointer v);
  Pointer metacall_value_count(Pointer v);
  void metacal_value_destroy(Pointer v);
  int metacall_value_id(Pointer v);


  // create instance of interface
  Bindings INSTANCE = (Bindings)Native.load("metacall", Bindings.class);
  boolean runningInMetacall = System.getProperty("metacall.polyglot.name") == "core";
}
