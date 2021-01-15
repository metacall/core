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
import java.nio.file.Paths

class size_t(value: Long) extends IntegerType(Native.SIZE_T_SIZE, value) {
  def this()  {
    this(0)
  }
}

trait MetaCallBindings extends Library {
  // metacall.h
  def metacall_initialize(): Int
  def metacall_load_from_file(tag: String, paths: Array[String], size: size_t, handle: Pointer): Int
  def metacallv_s(name: String, args: Array[Pointer], size: size_t): Pointer
  def metacall_destroy(): Int

  // metacall_value.h
  def metacall_value_create_int(i: Int): Pointer
  def metacall_value_create_string(str: String, length: size_t): Pointer

  def metacall_value_to_int(v: Pointer): Int
  def metacall_value_to_long(v: Pointer): Long
  def metacall_value_to_string(v: Pointer): String

  def metacall_value_from_int(v: Pointer, i: Int): Pointer
  def metacall_value_from_string(v: Pointer, str: String, length: size_t): Pointer

  def metacall_value_size(v: Pointer): size_t
  def metacall_value_count(v: Pointer): size_t

  def metacall_value_destroy(v: Pointer)

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

object MetaCall {
  var metacall = Native.load("metacall", classOf[MetaCallBindings]);

  // This is a experiment I have been doing in order to implement a high level
  // abstraction for Scala. The objective is to make it as trasparent as possible,
  // like if you were dealing with normal scala values. Generics can help for sure.

  /*
  trait ValueLifetime {
    def finalize()
  }

  def finalizer [T, V <: ValueLifetime] (v : V) (f : V => T) : T =
    try f(v)
    finally v.finalize()

  abstract class ValuePtr[T](v: Pointer) extends ValueLifetime {
    def finalize() {
      metacall.metacall_value_destroy(v)
    }
  }

  class Value[@specialized(Int) T](i: Int) extends ValuePtr[T](metacall.metacall_value_create_int(i)) {
    def to_value(): T = {
      metacall.metacall_value_to_int(v)
    }
  }

  class Value[@specialized(String) T](str: String) extends ValuePtr(metacall.metacall_value_create_string(i)) {

  }
  */

  if (metacall.metacall_initialize() != 0) {
    throw new RuntimeException("MetaCall could not initialize")
  }

  var paths = Array(
    Paths.get("./src/test/scala/scripts/main.py").toAbsolutePath.toString()
  )

  // Load the script list
  if (metacall.metacall_load_from_file("py", paths, new size_t(paths.length), null) != 0) {
    throw new RuntimeException("MetaCall failed to load the script")
  }

  // Create array of parameters
  var args = Array(
    metacall.metacall_value_create_int(3),
    metacall.metacall_value_create_int(5),
  )

  // Invoke the function
  var ret = metacall.metacallv_s("hello_sacala_from_python", args, new size_t(args.length))

  // Note: Python uses longs, so it returns a long value
  println("Result:", metacall.metacall_value_to_long(ret))

  // For avoiding conversion errors, it is possible to test against metacall_value_id,
  // or there is also a casting API for dealing with it.

  // Clear parameters
  args.foreach { metacall.metacall_value_destroy }

  // Clear return value
  metacall.metacall_value_destroy(ret)

  if (metacall.metacall_destroy() != 0) {
    throw new RuntimeException("MetaCall did not finish successfully")
  }
}
