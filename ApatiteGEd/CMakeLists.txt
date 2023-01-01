project(ApatiteGEd)

file(GLOB_RECURSE ApatiteGEd_SRC "*.cpp" "*.h")

add_library(${PROJECT_NAME} STATIC ${ApatiteGEd_SRC})