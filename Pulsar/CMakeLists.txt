cmake_minimum_required(VERSION 3.23)
project("PulsarEngine")

if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /Ob0 /Od /RTC1" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_DEBUG "/MDd /Zi /Ob0 /Od /RTC1" CACHE STRING "" FORCE)
endif()

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_subdirectory("Packages/Pulsar/Source")
add_subdirectory("Packages/PulsarEd/Source")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Packages/UnrealSceneImporter/CMakeLists.txt")
    add_subdirectory("Packages/UnrealSceneImporter")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Packages/Project/CMakeLists.txt")
    add_subdirectory("Packages/Project")
endif()
