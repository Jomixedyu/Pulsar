
set(PUBLIC_MODULE
        UnrealSceneImporter
)


foreach (module ${PUBLIC_MODULE})
    add_subdirectory(${PROJECT_NAME} ${module})
endforeach ()

