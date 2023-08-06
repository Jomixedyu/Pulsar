
project(jxcorlib.ser)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/utf-8")
endif()

add_library(jxcorlib.ser STATIC OBJECT "JsonSerializer.h" "JsonSerializer.cpp" "Assembly.h" "DataSerializer.h" "DataSerializer.cpp")
