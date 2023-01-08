
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/utf-8")
endif()

add_library(jxcorlib STATIC OBJECT "UString.cpp" "Object.cpp" "Type.cpp" "Assembly.cpp" "Reflection.cpp" "File.cpp"  "Converter.hpp" "Core.h"  "EnumUtil.h" "IInterface.h" "Delegate.h" "BasicTypes.h" "BasicTypes.cpp" "time_counter.h" "Enum.h" "Enum.cpp" "Guid.h" "Guid.cpp")