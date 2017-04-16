# CMake entry point
cmake_minimum_required (VERSION 2.8)
project (Tutorials)

option(USE_MINGW "Build with MinGW to windows" OFF)

find_package(OpenGL REQUIRED)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -flto ")

# Compile external dependencies 
add_subdirectory (external)

include_directories(
	external/glfw-3.1.2/include/GLFW/
	external/glm-0.9.7.1/
	external/glew-1.13.0/include/
	.
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	GLEW_1130
	#noise
	SDL2
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

add_executable(termination_shock
	game/main.cpp
	common/shader.cpp
	common/shader.hpp
	common/texture.cpp
	common/texture.hpp

        game/VoxelMapRenderer.h
        game/VoxelMapRenderer.cpp
        game/VoxelRenderMap.h
        game/VoxelRenderMap.cpp
        game/StarArray.h
        game/StarArray.cpp
        game/Entity.h
        game/Entity.cpp
        game/MovingEntity.h
        game/MovingEntity.cpp
        game/VoxelChunk.h
        game/VoxelChunk.cpp
        game/DeepSpaceRenderer.cpp
        game/DeepSpaceRenderer.h
        game/Rec.h
        game/Rec.cpp
        game/TexRecArray.h
        game/TexRecArray.cpp
        game/Texture.h
        game/Texture.cpp
        game/FPSCounter.h
        game/FPSCounter.cpp
        game/IngameInterface.h
        game/IngameInterface.cpp
	game/Map.cpp
	game/Map.h
	game/RenderWindow.cpp
	game/RenderWindow.h
	game/Camera.cpp
	game/Camera.h
	game/Controls.cpp
	game/Controls.h
	common/Common.h
	game/Voxel.cpp
	game/Voxel.h
	game/v3.cpp
	game/v3.h
	game/stb_perlin.cpp
)

target_link_libraries(termination_shock
	${ALL_LIBS}
)

SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$" )
