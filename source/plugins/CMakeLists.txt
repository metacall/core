
# Check if plugins are enabled
if(NOT OPTION_BUILD_PLUGINS)
    return()
endif()

# Plugin packages
#add_subdirectory(c_loader) # Foreing Function Interface library
#add_subdirectory(jsm_loader) # Mozilla's SpiderMonkey 4.8 JavaScript Engine
add_subdirectory(js_loader) # Google's V8 5.1 JavaScript Engine
add_subdirectory(py_loader) # Python 3.5 C API
add_subdirectory(rb_loader) # Ruby 2.3 C API
add_subdirectory(cs_loader) # CoreCRT 1.0.1 Runtime
add_subdirectory(mock_loader) # Mock plugin without dependencies (core testing)
