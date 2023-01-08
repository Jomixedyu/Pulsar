project(ApatiteGEd)

file(GLOB_RECURSE ApatiteGEd_SRC "*.cpp" "*.h")

add_library(${PROJECT_NAME} STATIC OBJECT ${ApatiteGEd_SRC})