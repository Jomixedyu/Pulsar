project(apatiteed)

file(GLOB_RECURSE ApatiteEd_SRC "*.cpp" "*.h")

add_library(${PROJECT_NAME} STATIC ${ApatiteEd_SRC})