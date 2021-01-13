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
 
trait MetaCall extends Library {
	def metacall_initialize(): Int
	def metacall_load_from_file(runtime: String, paths: Array[String]): Int
	def metacallv_s(name: String, args: Array[Pointer], size Long): Pointer

	// TODO: Implement value methods

	def metacall_destroy(): Int
}
 
object MetaCall {
	private var metacall : MetaCall = null;

	// Initialization block
	{
		if (metacall == null) {
			metacall = Native.load("metacall", classOf[MetaCall].asInstanceOf[MetaCall])
		}

		if (metacall.metacall_initialize() != 0) {
			throw new RuntimeException("MetaCall could not initialize")
		}
	}
}
