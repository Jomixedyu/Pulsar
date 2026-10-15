
project(astc)


add_library(astc "ASTC.h" "ASTC.cpp")

add_subdirectory("./astc-encoder")
target_include_directories(${PROJECT_NAME} PRIVATE "./astc-encoder/Source")