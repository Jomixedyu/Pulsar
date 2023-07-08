project(PulsarGEd)

file(GLOB_RECURSE PulsarGEd_SRC "*.cpp" "*.h")

add_library(${PROJECT_NAME} STATIC OBJECT ${PulsarGEd_SRC})