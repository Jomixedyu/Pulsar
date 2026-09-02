

set(BOX2D_BUILD_UNIT_TESTS OFF)
set(BOX2D_BUILD_TESTBED OFF)
add_subdirectory("./box2d")

target_include_directories(${PROJECT_NAME} PRIVATE "./box2d/include")
