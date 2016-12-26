
#
# Post generation step (support for variadic arguments)
#

set(js_port_args_check "if(args.Length() != 1) SWIG_exception_fail(SWIG_ERROR, \"Illegal number of arguments for _wrap_metacall.\");")
set(js_port_post_generated_file "${SWIG_GENERATED_FILE}.post")

file(READ "${SWIG_GENERATED_FILE}" generated_file)

string(REGEX REPLACE ";" "\\\\;" generated_file "${generated_file}")
string(REPLACE "${js_port_args_check}" " " output_generated_file ${generated_file})

file(WRITE ${OUTPUT_GENERATED_FILE} "${output_generated_file}")
