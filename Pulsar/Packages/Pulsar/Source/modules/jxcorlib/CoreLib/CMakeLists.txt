
project ("jxcorlib.core")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/utf-8")
endif()

aux_source_directory(./ JXCORELIB_DIR_SRCS)

add_library(${PROJECT_NAME} STATIC ${JXCORELIB_DIR_SRCS})