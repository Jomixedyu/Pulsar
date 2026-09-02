cmake_minimum_required(VERSION 3.23)

if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    project(angelscript LANGUAGES CXX ASM_MASM)
else()
    project(angelscript LANGUAGES CXX)
endif()

set(AS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(AS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Core AngelScript source files (platform-agnostic)
file(GLOB AS_CORE_SOURCES
    ${AS_SOURCE_DIR}/as_atomic.cpp
    ${AS_SOURCE_DIR}/as_builder.cpp
    ${AS_SOURCE_DIR}/as_bytecode.cpp
    ${AS_SOURCE_DIR}/as_callfunc.cpp
    ${AS_SOURCE_DIR}/as_compiler.cpp
    ${AS_SOURCE_DIR}/as_configgroup.cpp
    ${AS_SOURCE_DIR}/as_context.cpp
    ${AS_SOURCE_DIR}/as_datatype.cpp
    ${AS_SOURCE_DIR}/as_gc.cpp
    ${AS_SOURCE_DIR}/as_generic.cpp
    ${AS_SOURCE_DIR}/as_globalproperty.cpp
    ${AS_SOURCE_DIR}/as_memory.cpp
    ${AS_SOURCE_DIR}/as_module.cpp
    ${AS_SOURCE_DIR}/as_objecttype.cpp
    ${AS_SOURCE_DIR}/as_outputbuffer.cpp
    ${AS_SOURCE_DIR}/as_parser.cpp
    ${AS_SOURCE_DIR}/as_restore.cpp
    ${AS_SOURCE_DIR}/as_scriptcode.cpp
    ${AS_SOURCE_DIR}/as_scriptengine.cpp
    ${AS_SOURCE_DIR}/as_scriptfunction.cpp
    ${AS_SOURCE_DIR}/as_scriptnode.cpp
    ${AS_SOURCE_DIR}/as_scriptobject.cpp
    ${AS_SOURCE_DIR}/as_string.cpp
    ${AS_SOURCE_DIR}/as_string_util.cpp
    ${AS_SOURCE_DIR}/as_thread.cpp
    ${AS_SOURCE_DIR}/as_tokenizer.cpp
    ${AS_SOURCE_DIR}/as_typeinfo.cpp
    ${AS_SOURCE_DIR}/as_variablescope.cpp
)

# Platform-specific callfunc implementations
if(MSVC)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        list(APPEND AS_CORE_SOURCES ${AS_SOURCE_DIR}/as_callfunc_x64_msvc.cpp)
        list(APPEND AS_CORE_SOURCES ${AS_SOURCE_DIR}/as_callfunc_x64_msvc_asm.asm)
    else()
        list(APPEND AS_CORE_SOURCES ${AS_SOURCE_DIR}/as_callfunc_x86.cpp)
    endif()
elseif(MINGW)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        list(APPEND AS_CORE_SOURCES ${AS_SOURCE_DIR}/as_callfunc_x64_mingw.cpp)
    else()
        list(APPEND AS_CORE_SOURCES ${AS_SOURCE_DIR}/as_callfunc_x86.cpp)
    endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    list(APPEND AS_CORE_SOURCES ${AS_SOURCE_DIR}/as_callfunc_arm64.cpp)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    list(APPEND AS_CORE_SOURCES ${AS_SOURCE_DIR}/as_callfunc_arm.cpp)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "riscv64")
    list(APPEND AS_CORE_SOURCES ${AS_SOURCE_DIR}/as_callfunc_riscv64.cpp)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ppc|powerpc")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        list(APPEND AS_CORE_SOURCES ${AS_SOURCE_DIR}/as_callfunc_ppc_64.cpp)
    else()
        list(APPEND AS_CORE_SOURCES ${AS_SOURCE_DIR}/as_callfunc_ppc.cpp)
    endif()
else()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        list(APPEND AS_CORE_SOURCES ${AS_SOURCE_DIR}/as_callfunc_x64_gcc.cpp)
    else()
        list(APPEND AS_CORE_SOURCES ${AS_SOURCE_DIR}/as_callfunc_x86.cpp)
    endif()
endif()

# Add-ons commonly used with the engine
set(AS_ADDON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/add_on)

set(AS_ADDON_SOURCES
    ${AS_ADDON_DIR}/scriptarray/scriptarray.cpp
    ${AS_ADDON_DIR}/scriptbuilder/scriptbuilder.cpp
    ${AS_ADDON_DIR}/scriptdictionary/scriptdictionary.cpp
    ${AS_ADDON_DIR}/scripthandle/scripthandle.cpp
    ${AS_ADDON_DIR}/scripthelper/scripthelper.cpp
    ${AS_ADDON_DIR}/scriptmath/scriptmath.cpp
    ${AS_ADDON_DIR}/scriptstdstring/scriptstdstring.cpp
    ${AS_ADDON_DIR}/scriptstdstring/scriptstdstring_utils.cpp
)

add_library(angelscript STATIC ${AS_CORE_SOURCES} ${AS_ADDON_SOURCES})

target_include_directories(angelscript PUBLIC
    ${AS_INCLUDE_DIR}
    ${AS_ADDON_DIR}
)

# AngelScript is typically compiled without exceptions in game engines,
# but we keep the default for now.
target_compile_features(angelscript PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(angelscript PRIVATE /W0)
else()
    target_compile_options(angelscript PRIVATE -w)
endif()

# Disable some noisy warnings on MSVC
if(MSVC)
    target_compile_options(angelscript PRIVATE
        /wd4996  # CRT deprecation warnings
    )
endif()
