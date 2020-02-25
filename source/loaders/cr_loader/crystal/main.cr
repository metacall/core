#
#	Loader Library by Parra Studios
#	A plugin for loading crystal code at run-time into a process.
#
#	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#		http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.
#

lib LibCrystalMain
	@[Raises]
	fun __crystal_main(argc : Int32, argv : UInt8**)
end

module Crystal
	@@stdin_is_blocking = false
	@@stdout_is_blocking = false
	@@stderr_is_blocking = false

	# Defines the main routine run by normal Crystal programs:
	#
	# - Initializes the GC
	# - Invokes the given *block*
	# - Handles unhandled exceptions
	# - Invokes `at_exit` handlers
	# - Flushes `STDOUT` and `STDERR`
	#
	# This method can be invoked if you need to define a custom
	# main (as in C main) function, doing all the above steps.
	#
	# For example:
	#
	# ```
	# fun main(argc : Int32, argv : UInt8**) : Int32
	# 	Crystal.main do
	# 		elapsed = Time.measure do
	# 			Crystal.main_user_code(argc, argv)
	# 		end
	# 		puts "Time to execute program: #{elapsed}"
	# 	end
	# end
	# ```
	#
	# Note that the above is really just an example, almost the
	# same can be accomplished with `at_exit`. But in some cases
	# redefinition of C's main is needed.
	def self.main(&block)
		# TODO: There is no GC.destroy method so we should implement a mechanism
		# to reinitialize the GC. A way of doing it maybe calling GC.init only once (like a singleton)
		# then using GC.collect and GC.disable when destroying, and next time initialize is called,
		# use GC.enable instead of GC.init
		GC.init

		remember_blocking_state

		status =
			begin
				yield
				0
			rescue ex
				1
			end

		status = AtExitHandlers.run status, ex
		ignore_stdio_errors { STDOUT.flush }
		ignore_stdio_errors { STDERR.flush }

		restore_blocking_state

		raise ex if ex
		status
	end

	# :nodoc:
	def self.ignore_stdio_errors
		yield
	rescue IO::Error
	rescue Errno
	end

	# :nodoc:
	def self.remember_blocking_state
		@@stdin_is_blocking = IO::FileDescriptor.fcntl(0, LibC::F_GETFL) & LibC::O_NONBLOCK == 0
		@@stdout_is_blocking = IO::FileDescriptor.fcntl(1, LibC::F_GETFL) & LibC::O_NONBLOCK == 0
		@@stderr_is_blocking = IO::FileDescriptor.fcntl(2, LibC::F_GETFL) & LibC::O_NONBLOCK == 0
	end

	# :nodoc:
	def self.restore_blocking_state
		STDIN.blocking = @@stdin_is_blocking
		STDOUT.blocking = @@stdout_is_blocking
		STDERR.blocking = @@stderr_is_blocking
	end

	# Main method run by all Crystal programs at startup.
	#
	# This setups up the GC, invokes your program, rescuing
	# any handled exception, and then runs `at_exit` handlers.
	#
	# This method is automatically invoked for you, so you
	# don't need to invoke it.
	#
	# However, if you need to define a special main C function,
	# you can redefine main and invoke `Crystal.main` from it:
	#
	# ```
	# fun main(argc : Int32, argv : UInt8**) : Int32
	# 	# some setup before Crystal main
	# 	Crystal.main(argc, argv)
	# 	# some cleanup logic after Crystal main
	# end
	# ```
	#
	# The `Crystal.main` can also be passed as a callback:
	#
	# ```
	# fun main(argc : Int32, argv : UInt8**) : Int32
	# 	LibFoo.init_foo_and_invoke_main(argc, argv, ->Crystal.main)
	# end
	# ```
	#
	# Note that before `Crystal.main` is invoked the GC
	# is not setup yet, so nothing that allocates memory
	# in Crystal (like `new` for classes) can be used.
	def self.main(argc : Int32, argv : UInt8**)
		main do
			main_user_code(argc, argv)
		end
	rescue ex
		Crystal::System.print_exception "Unhandled exception", ex
		1
	end

	# Executes the main user code. This normally is executed
	# after initializing the GC and before executing `at_exit` handlers.
	#
	# You should never invoke this method unless you need to
	# redefine C's main function. See `Crystal.main` for
	# more details.
	def self.main_user_code(argc : Int32, argv : UInt8**)
		LibCrystalMain.__crystal_main(argc, argv)
	end
end

# Main function that acts as C's main function.
# Invokes `Crystal.main`.
#
# Can be redefined. See `Crystal.main` for examples.
fun main = crystal_library_init(argc : Int32, argv : UInt8**) : Int32
	Crystal.main(argc, argv)
end
