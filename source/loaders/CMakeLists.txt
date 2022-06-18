# Check if loaders are enabled
if(NOT OPTION_BUILD_LOADERS)
	return()
endif()

# Plugins options
option(OPTION_BUILD_LOADERS_C "Build C Foreign Function Interface library loader plugin." OFF)
option(OPTION_BUILD_LOADERS_COB "Build GNU/COBOL 2.2 loader plugin." OFF)
option(OPTION_BUILD_LOADERS_CR "Build Crystal 0.33.0 loader plugin." OFF)
option(OPTION_BUILD_LOADERS_CS "Build C# CoreCLR 5.0.17 Runtime loader plugin." OFF)
option(OPTION_BUILD_LOADERS_DART "Build Dart VM 2.8.4 Runtime loader plugin." OFF)
option(OPTION_BUILD_LOADERS_EXT "Build MetaCall Core Extension loader plugin." ON)
option(OPTION_BUILD_LOADERS_FILE "Build File System loader plugin." OFF)
option(OPTION_BUILD_LOADERS_JAVA "Build Java Virtual Machine loader plugin." OFF)
option(OPTION_BUILD_LOADERS_JL "Build Julia 1.6 loader plugin." OFF)
option(OPTION_BUILD_LOADERS_JSM "Build JavaScript SpiderMonkey 4.8 loader plugin." OFF)
option(OPTION_BUILD_LOADERS_JS "Build JavaScript V8 5.1+ loader plugin." OFF)
option(OPTION_BUILD_LOADERS_LLVM "Build LLVM 11 loader plugin." OFF)
option(OPTION_BUILD_LOADERS_LUA "Build LuaJIT2 v2.1 (OpenResty fork) loader plugin." OFF)
option(OPTION_BUILD_LOADERS_MOCK "Build mock loader loader plugin." ON)
option(OPTION_BUILD_LOADERS_NODE "Build NodeJS v12.21.0 JavaScript Runtime loader plugin." OFF)
option(OPTION_BUILD_LOADERS_PY "Build Python 3.9 C API loader plugin." OFF)
option(OPTION_BUILD_LOADERS_RB "Build Ruby 2.7 C API loader plugin." OFF)
option(OPTION_BUILD_LOADERS_RS "Build Rust 1.55.0 loader plugin." OFF)
option(OPTION_BUILD_LOADERS_RPC "Build cURL Remote Procedure Call loader plugin." OFF)
option(OPTION_BUILD_LOADERS_TS "Build TypeScript 3.9.7 Runtime loader plugin." OFF)
option(OPTION_BUILD_LOADERS_WASM "Build WebAssembly Virtual Machine loader plugin." OFF)

# Plugin packages
add_subdirectory(c_loader) # Foreign Function Interface library
add_subdirectory(cob_loader) # GNU/Cobol 2.2 Runtime
add_subdirectory(cr_loader) # Crystal 0.33.0 Runtime
add_subdirectory(cs_loader) # CoreCLR 5.0.17 Runtime
add_subdirectory(dart_loader) # Dart VM 2.8.4 Runtime
add_subdirectory(ext_loader) # MetaCall Core Extensions
add_subdirectory(file_loader) # File System
add_subdirectory(java_loader) # Java Virtual Machine
add_subdirectory(jl_loader) # Julia Runtime
add_subdirectory(jsm_loader) # Mozilla's SpiderMonkey 4.8 JavaScript Engine
add_subdirectory(js_loader) # Google's V8 5.1 JavaScript Engine
add_subdirectory(llvm_loader) # LLVM Compiler Infrastructure
add_subdirectory(lua_loader) # LuaJIT2 v2.1 (OpenResty fork) Runtime
add_subdirectory(mock_loader) # Mock loader plugin without dependencies (core testing)
add_subdirectory(node_loader) # NodeJS v12.21.0 JavaScript Runtime
add_subdirectory(py_loader) # Python 3.9 C API
add_subdirectory(rb_loader) # Ruby 2.7 C API
add_subdirectory(rs_loader) # Rust 1.55.0
add_subdirectory(rpc_loader) # cURL Remote Procedure Call
add_subdirectory(ts_loader) # TypeScript 3.9.7
add_subdirectory(wasm_loader) # WebAssembly Virtual Machine

# Optionally enable loader dependencies utility
macro(add_loader_dependencies target)
	set(loaders_map_c_loader	OPTION_BUILD_LOADERS_C)
	set(loaders_map_cob_loader	OPTION_BUILD_LOADERS_COB)
	set(loaders_map_cr_loader	OPTION_BUILD_LOADERS_CR)
	set(loaders_map_cs_loader	OPTION_BUILD_LOADERS_DART)
	set(loaders_map_dart_loader	OPTION_BUILD_LOADERS_CS)
	set(loaders_map_ext_loader	OPTION_BUILD_LOADERS_EXT)
	set(loaders_map_file_loader	OPTION_BUILD_LOADERS_FILE)
	set(loaders_map_java_loader	OPTION_BUILD_LOADERS_JAVA)
	set(loaders_map_jl_loader	OPTION_BUILD_LOADERS_JL)
	set(loaders_map_jsm_loader	OPTION_BUILD_LOADERS_JSM)
	set(loaders_map_js_loader	OPTION_BUILD_LOADERS_JS)
	set(loaders_map_llvm_loader	OPTION_BUILD_LOADERS_LLVM)
	set(loaders_map_lua_loader	OPTION_BUILD_LOADERS_LUA)
	set(loaders_map_mock_loader	OPTION_BUILD_LOADERS_MOCK)
	set(loaders_map_node_loader	OPTION_BUILD_LOADERS_NODE)
	set(loaders_map_py_loader	OPTION_BUILD_LOADERS_PY)
	set(loaders_map_rb_loader	OPTION_BUILD_LOADERS_RB)
	set(loaders_map_rs_loader	OPTION_BUILD_LOADERS_RS)
	set(loaders_map_rpc_loader	OPTION_BUILD_LOADERS_RPC)
	set(loaders_map_ts_loader	OPTION_BUILD_LOADERS_TS)
	set(loaders_map_wasm_loader	OPTION_BUILD_LOADERS_WASM)

	set(LOADERS_LIST)

	foreach(loader ${ARGN})
		if(${loaders_map_${loader}})
			set(LOADERS_LIST
				${LOADERS_LIST}
				${loader}
			)
		endif()
	endforeach()

	if(LOADERS_LIST)
		add_dependencies(${target} ${LOADERS_LIST})
	endif()
endmacro()
