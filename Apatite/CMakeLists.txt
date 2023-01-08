project(apatite)

file(GLOB_RECURSE Apatite_SRC "*.cpp" "*.h")

add_library(${PROJECT_NAME} STATIC OBJECT ${Apatite_SRC})