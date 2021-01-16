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
import util._

/** Interface mirroring the MetaCall library using JNA. See:
  * - http://java-native-access.github.io/jna/5.6.0/javadoc/overview-summary.html
  * - https://github.com/metacall/core/blob/develop/source/metacall/include/metacall/metacall.h
  * - https://github.com/metacall/core/blob/develop/source/metacall/include/metacall/metacall_value.h
  */
protected[metacall] trait Bindings extends Library {
  // metacall.h
  def metacall_initialize(): Int

  def metacall_load_from_file(
      tag: String,
      paths: Array[String],
      size: SizeT,
      handle: Pointer
  ): Int

  def metacallv_s(name: String, args: Array[Pointer], size: SizeT): Pointer

  def metacall_destroy(): Int

  // metacall_value.h
  def metacall_value_create_int(i: Int): Pointer

  def metacall_value_create_string(str: String, length: SizeT): Pointer

  def metacall_value_create_double(v: Double): Pointer

  def metacall_value_create_array(
      values: Array[Pointer],
      valuesSize: SizeT
  ): Pointer

  def metacall_value_create_map(tuples: Array[Pointer], size: SizeT): Pointer

  def metacall_value_to_int(v: Pointer): Int

  def metacall_value_to_long(v: Pointer): Long

  def metacall_value_to_string(v: Pointer): String

  def metacall_value_to_array(v: Pointer): Array[Pointer]

  def metacall_value_to_map(v: Pointer): Array[Pointer]

  def metacall_value_from_int(v: Pointer, i: Int): Pointer

  def metacall_value_from_string(
      v: Pointer,
      str: String,
      length: SizeT
  ): Pointer

  def metacall_value_size(v: Pointer): SizeT

  def metacall_value_count(v: Pointer): SizeT

  def metacall_value_destroy(v: Pointer): Unit

  // TODO:
  /*
  enum metacall_value_id
  {
    METACALL_BOOL		= 0,
    METACALL_CHAR		= 1,
    METACALL_SHORT		= 2,
    METACALL_INT		= 3,
    METACALL_LONG		= 4,
    METACALL_FLOAT		= 5,
    METACALL_DOUBLE		= 6,
    METACALL_STRING		= 7,
    METACALL_BUFFER		= 8,
    METACALL_ARRAY		= 9,
    METACALL_MAP		= 10,
    METACALL_PTR		= 11,
    METACALL_FUTURE		= 12,
    METACALL_FUNCTION	= 13,
    METACALL_NULL		= 14,
    METACALL_CLASS		= 15,
    METACALL_OBJECT		= 16,

    METACALL_SIZE,
    METACALL_INVALID
  };
   */

  // TODO:
  def metacall_value_id(v: Pointer): Int /* enum metacall_value_id */
}
private[metacall] object Bindings {
  val instance = Native.load("metacall", classOf[Bindings])
}
