cmake_minimum_required(VERSION 3.11.1 FATAL_ERROR)

project(Simple-Physics)

include_directories("${CMAKE_SOURCE_DIR}/inc"
                    "${CMAKE_SOURCE_DIR}/dependencies/cgltf"
                    "${CMAKE_SOURCE_DIR}/dependencies/glad"
                    "${CMAKE_SOURCE_DIR}/dependencies/GLFW"
                    "${CMAKE_SOURCE_DIR}/dependencies/glm"
                    "${CMAKE_SOURCE_DIR}/dependencies/imgui"
                    "${CMAKE_SOURCE_DIR}/dependencies/KHR"
                    "${CMAKE_SOURCE_DIR}/dependencies/stb_image")

set(project_headers
    inc/AnimatedMesh.h
    inc/Camera3.h
    inc/Decal.h
    inc/DecalRenderer.h
    inc/FiniteStateMachine.h
    inc/Frame.h
    inc/Game.h
    inc/GLTFLoader.h
    inc/Interpolation.h
    inc/PlayState.h
    inc/Quat.h
    inc/ResourceManager.h
    inc/Shader.h
    inc/ShaderLoader.h
    inc/SimpleMesh.h
    inc/State.h
    inc/Texture.h
    inc/TextureLoader.h
    inc/Track.h
    inc/Transform.h
    inc/Window.h)

set(project_sources
    src/AnimatedMesh.cpp
    src/Camera3.cpp
    src/Decal.cpp
    src/DecalRenderer.cpp
    src/FiniteStateMachine.cpp
    src/Game.cpp
    src/GLTFLoader.cpp
    src/main.cpp
    src/PlayState.cpp
    src/Quat.cpp
    src/Shader.cpp
    src/ShaderLoader.cpp
    src/SimpleMesh.cpp
    src/Texture.cpp
    src/TextureLoader.cpp
    src/Track.cpp
    src/Transform.cpp
    src/Window.cpp
    dependencies/cgltf/cgltf/cgltf.c
    dependencies/imgui/imgui/imgui.cpp
    dependencies/imgui/imgui/imgui_demo.cpp
    dependencies/imgui/imgui/imgui_draw.cpp
    dependencies/imgui/imgui/imgui_impl_glfw.cpp
    dependencies/imgui/imgui/imgui_impl_opengl3.cpp
    dependencies/imgui/imgui/imgui_tables.cpp
    dependencies/imgui/imgui/imgui_widgets.cpp
    dependencies/stb_image/stb_image/stb_image.cpp)

# This path must be relative to the location of the build folder
set(project_resources "../resources@resources")

set(CMAKE_EXECUTABLE_SUFFIX ".html")

# For debugging
#set(CMAKE_CXX_FLAGS "-O3 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s WASM=1 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1 -o index.html --preload-file ${project_resources} --use-preload-plugins")
# For releasing
set(CMAKE_CXX_FLAGS "-O3 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -o index.html --preload-file ${project_resources} --use-preload-plugins")

add_executable(${PROJECT_NAME} ${project_headers} ${project_sources})
