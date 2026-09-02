


option(${PROJECT_NAME}_BUILD_EXECUTABLE "enable executable mode" OFF)

list(APPEND PSC_LIST "ShaderCompiler.cpp" "GlslangCompilerImpl.h" "ShaderReflect.cpp")

if(${PROJECT_NAME}_BUILD_EXECUTABLE)
#list(APPEND PSC_LIST "main.cpp")
add_executable (${PROJECT_NAME} ${PSC_LIST})
else()
add_library(${PROJECT_NAME} STATIC ${PSC_LIST})

if(MSVC)
    target_link_options(${PROJECT_NAME} INTERFACE "/WHOLEARCHIVE:$<TARGET_FILE:${PROJECT_NAME}>")
endif()
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE API_EXPORTS)
