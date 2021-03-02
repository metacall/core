/*
 *	MetaCall Scala Port by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

package metacall

import com.sun.jna._
import com.sun.jna.ptr.PointerByReference

import util._

/** Interface mirroring the MetaCall library using JNA. See:
  * - http://java-native-access.github.io/jna/5.6.0/javadoc/overview-summary.html
  * - https://github.com/metacall/core/blob/develop/source/metacall/include/metacall/metacall.h
  * - https://github.com/metacall/core/blob/develop/source/metacall/include/metacall/metacall_value.h
  */
protected[metacall] trait Bindings extends Library {
  // metacall.h
  def metacall_initialize(): Int

  /** Should be called before `metacall_initialize` */
  def metacall_log_null(): Unit

  def metacall_load_from_file(
      tag: String,
      paths: Array[String],
      size: SizeT,
      handle: PointerByReference
  ): Int

  def metacall_handle_export(handle: Pointer): Pointer

  def metacallv_s(name: String, args: Array[Pointer], size: SizeT): Pointer
  def metacallfv_s(func: Pointer, args: Array[Pointer], size: SizeT): Pointer

  trait ResolveCallback extends Callback {
    def invoke(result: Pointer, data: Pointer): Pointer
  }

  trait RejectCallback extends Callback {
    def invoke(error: Pointer, data: Pointer): Pointer
  }

  def metacall_await_s(
      name: String,
      args: Array[Pointer],
      size: SizeT,
      resolve: ResolveCallback,
      reject: RejectCallback,
      data: Pointer
  ): Pointer

  def metacall_await_future(
      future: Pointer,
      reslove: ResolveCallback,
      reject: RejectCallback,
      data: Pointer
  ): Pointer

  def metacallhv_s(
      handle: Pointer,
      name: String,
      args: Array[Pointer],
      size: SizeT
  ): Pointer

  def metacall_register(
      name: String,
      invoke: FunctionPointer,
      func: PointerByReference,
      ret: Int,
      size: SizeT,
      types: Array[Int]
  ): Int

  def metacall_function(name: String): Pointer

  def metacall_function_size(func: Pointer): SizeT

  def metacall_function_async(func: Pointer): Int

  def metacall_destroy(): Int

  // metacall_value.h
  def metacall_value_create_int(i: Int): Pointer
  def metacall_value_create_long(l: Long): Pointer
  def metacall_value_create_short(s: Short): Pointer
  def metacall_value_create_string(str: String, length: SizeT): Pointer
  def metacall_value_create_char(c: Char): Pointer
  def metacall_value_create_double(d: Double): Pointer
  def metacall_value_create_float(f: Float): Pointer
  def metacall_value_create_bool(b: Boolean): Pointer
  def metacall_value_create_function(f: Pointer): Pointer
  def metacall_value_create_array(
      values: Array[Pointer],
      size: SizeT
  ): Pointer
  def metacall_value_create_map(tuples: Array[Pointer], size: SizeT): Pointer
  def metacall_value_create_null(): Pointer

  def metacall_value_to_int(v: Pointer): Int
  def metacall_value_to_long(v: Pointer): Long
  def metacall_value_to_short(v: Pointer): Short
  def metacall_value_to_float(v: Pointer): Float
  def metacall_value_to_double(v: Pointer): Double
  def metacall_value_to_bool(v: Pointer): Boolean
  def metacall_value_to_char(v: Pointer): Char
  def metacall_value_to_string(v: Pointer): String
  def metacall_value_to_array(v: Pointer): Array[Pointer]
  def metacall_value_to_null(v: Pointer): Null
  def metacall_value_to_function(v: Pointer): Pointer
  def metacall_value_to_map(v: Pointer): Array[Pointer]
  def metacall_value_to_future(v: Pointer): Pointer

  def metacall_value_from_int(v: Pointer, i: Int): Pointer
  def metacall_value_from_string(
      v: Pointer,
      str: String,
      length: SizeT
  ): Pointer

  /** Returns the size of the value in bytes
    *  @param v Reference to the value
    *  @return Size in bytes of the value
    */
  def metacall_value_size(v: Pointer): SizeT // in bytes

  /** Returns the amount of values this value contains
    *  @param v Reference to the value
    *  @return Number of values @v represents
    */
  def metacall_value_count(v: Pointer): SizeT

  def metacall_value_copy(v: Pointer): Pointer

  def metacall_value_destroy(v: Pointer): Unit

  // TODO: Enhance return value using http://technofovea.com/blog/archives/815
  def metacall_value_id(v: Pointer): Int /* enum metacall_value_id */
}
private[metacall] object Bindings {
  val instance = Native.load("metacall", classOf[Bindings])

  val runningInMetacall = System.getProperty("metacall.polyglot.name") == "core"
}
