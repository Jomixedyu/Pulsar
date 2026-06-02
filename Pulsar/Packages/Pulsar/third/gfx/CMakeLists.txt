
project ("gfx")

file(GLOB_RECURSE gfx_SRC "include/*.h" "src/*.h" "src/*.cpp")

add_library(${PROJECT_NAME} STATIC OBJECT ${gfx_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC "include")