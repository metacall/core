package metacall

import java.util.*;
import java.io.*;

import com.sun.jna.*;
import com.sun.jna.PointerByReference;

protected metacall extends Library
{
  int metacall_initialize();

  void metacall_log_null();

  int metacall_load_from_file(String tag, String paths[], long size, PointerByReference handle);

  Pointer metacall_handle_export(Pointer handle);

  Pointer metacallv_s(String name, Pointer args[], long size);
  Pointer metacallfv_s(Pointer func, Pointer args[], long size);


}
