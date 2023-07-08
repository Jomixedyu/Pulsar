project(pulsared)

file(GLOB_RECURSE Pulsared_SRC "*.cpp" "*.h")


add_library(${PROJECT_NAME} STATIC OBJECT ${Pulsared_SRC})