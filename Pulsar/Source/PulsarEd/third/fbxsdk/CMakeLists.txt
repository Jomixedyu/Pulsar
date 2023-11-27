project("fbxsdk")

file(GLOB_RECURSE FBXSDK_SRC "src/*.c" "src/*.h" "src/*.hpp" "src/*.cpp" "include/*.h" "include/*.hpp")
add_library(fbxsdk INTERFACE ${FBXSDK_SRC})

target_include_directories(${PROJECT_NAME} INTERFACE "./include")
target_link_directories(${PROJECT_NAME} INTERFACE "./lib/x64/$<IF:$<CONFIG:Debug>,debug,release>")
target_link_libraries(${PROJECT_NAME} INTERFACE "libfbxsdk-md" "wininet" "libxml2-md" "zlib-md")