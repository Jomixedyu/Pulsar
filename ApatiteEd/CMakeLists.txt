project(apatiteed)

file(GLOB_RECURSE ApatiteEd_SRC "*.cpp" "*.h")


add_library(${PROJECT_NAME} STATIC OBJECT ${ApatiteEd_SRC})