
project(Example)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/utf-8")
endif()

include_directories("${PROJECT_SOURCE_DIR}")

aux_source_directory(./ EXAMPLE_SRC_DIR)
add_library(Example STATIC OBJECT ${EXAMPLE_SRC_DIR})

