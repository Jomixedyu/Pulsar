
project("UnrealSceneImporter")

file(GLOB_RECURSE UnrealSceneImporter_Src "include/*.h" "src/*.cpp")
add_library(${PROJECT_NAME} UnrealSceneImporter_Src)

