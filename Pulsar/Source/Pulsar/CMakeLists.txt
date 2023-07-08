project(pulsar)

file(GLOB_RECURSE Pulsar_SRC "*.cpp" "*.h")

add_library(${PROJECT_NAME} STATIC OBJECT ${Pulsar_SRC})