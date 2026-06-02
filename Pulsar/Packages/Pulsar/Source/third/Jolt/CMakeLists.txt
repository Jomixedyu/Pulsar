

cmake_policy(SET CMP0091 NEW)

set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF)
set(CPP_RTTI_ENABLED ON)
set(ENABLE_INSTALL OFF)

add_subdirectory(./JoltPhysics/Build)
