
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/utf-8")
endif()

add_library(jxcorlib.ser "JsonSerializer.cpp" "Assembly.h"  "DataSerializer.cpp")