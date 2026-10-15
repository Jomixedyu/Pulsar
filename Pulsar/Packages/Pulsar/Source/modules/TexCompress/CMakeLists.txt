set(BUILD_TOOLS OFF CACHE BOOL TexCompress FORCE)
set(BUILD_SAMPLE OFF CACHE BOOL TexCompress FORCE)
set(BUILD_DX11 ON CACHE BOOL TexCompress FORCE)
set(BUILD_DX12 OFF CACHE BOOL TexCompress FORCE)
set(ASTCENC_CLI OFF CACHE BOOL TexCompress FORCE)

add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/third/DirectXTex/DirectXTex
    ${CMAKE_CURRENT_BINARY_DIR}/third/DirectXTex/DirectXTex)
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/third/astc/astc-encoder
    ${CMAKE_CURRENT_BINARY_DIR}/third/astc/astc-encoder)

set_target_properties(astcenc-native-static PROPERTIES
    MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>DLL)

add_library(TexCompress STATIC
    TexCompress.cpp
)

target_include_directories(TexCompress PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(TexCompress PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third/DirectXTex/DirectXTex/DirectXTex
    ${CMAKE_CURRENT_SOURCE_DIR}/third/astc/astc-encoder/Source)
target_link_libraries(TexCompress PRIVATE
    $<LINK_ONLY:DirectXTex>
    $<LINK_ONLY:astcenc-native-static>
    d3d11)
