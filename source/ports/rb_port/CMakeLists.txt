# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_RB OR NOT OPTION_BUILD_LOADERS_RB)
	return()
endif()

#
# External dependencies
#

find_package(Ruby)

if(NOT Ruby_FOUND)
	message(SEND_ERROR "Ruby libraries not found")
	return()
endif()

#
# Port name and options
#

# Target name
set(target rb_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")





#
# Define test (CLI)
#

set(rb_port_test "${target}_test")
set(rb_port_test_path "${CMAKE_CURRENT_SOURCE_DIR}/test/run.rb")

if(OPTION_BUILD_CLI)
	message(STATUS "Test ${rb_port_test}")

	add_test(NAME ${rb_port_test}
		COMMAND $<TARGET_FILE:metacallcli> "${rb_port_test_path}"
	)

	#
	# Define test labels
	#

	set_property(TEST ${rb_port_test}
		PROPERTY LABELS ${rb_port_test}
	)

	include(TestEnvironmentVariables)

	test_environment_variables(${rb_port_test}
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)
endif()





# TODO

# #
# # Define test
# #

# if(OPTION_BUILD_ADDRESS_SANITIZER OR OPTION_BUILD_THREAD_SANITIZER)
# 	# TODO: This test fails when run with sanitizers:
# 	#
# 	# Sanitizer:
# 	# ASan runtime does not come first in initial library list; you should either link runtime to your application or manually preload it with LD_PRELOAD.
# 	# For solving the issue compile the test with sanitizers when enabled or preload asan
# 	#
# 	# Thread Sanitizer:
# 	# /usr/local/metacall/build/rb_port_test.rb:4:in `require_relative': /usr/lib/x86_64-linux-gnu/libtsan.so.2: cannot allocate memory in static TLS block - /usr/local/metacall/build/rb_portd.so (LoadError)
# 	# from /usr/local/metacall/build/rb_port_test.rb:4:in `<main>'
# 	return()
# endif()

# set(rb_port_test "rb_port_test")
# set(rb_port_test_path "${CMAKE_CURRENT_SOURCE_DIR}/test/run.rb")

# add_test(NAME ${target}
# 	COMMAND ${Ruby_EXECUTABLE} "${rb_port_test_path}"
# )

# #
# # Define test labels
# #

# set_property(TEST ${target}
# 	PROPERTY LABELS ${rb_port_test}
# )

# include(TestEnvironmentVariables)

# test_environment_variables(${target}
# 	""
# 	${TESTS_ENVIRONMENT_VARIABLES}
# )
