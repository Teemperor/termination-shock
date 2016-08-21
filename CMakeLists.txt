# CMake entry point
cmake_minimum_required (VERSION 2.8)
project (Tutorials)

option(USE_MINGW "Build with MinGW to windows" OFF)

find_package(OpenGL REQUIRED)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

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

add_executable(tutorial06_keyboard_and_mouse
	tutorial06_keyboard_and_mouse/tutorial06.cpp
	common/shader.cpp
	common/shader.hpp
	common/texture.cpp
	common/texture.hpp

	tutorial06_keyboard_and_mouse/Map.cpp
	tutorial06_keyboard_and_mouse/Map.h
	tutorial06_keyboard_and_mouse/RenderWindow.cpp
	tutorial06_keyboard_and_mouse/RenderWindow.h
	tutorial06_keyboard_and_mouse/Camera.cpp
	tutorial06_keyboard_and_mouse/Camera.h
	tutorial06_keyboard_and_mouse/Controls.cpp
	tutorial06_keyboard_and_mouse/Controls.h
		common/Common.h)

target_link_libraries(tutorial06_keyboard_and_mouse
	${ALL_LIBS}
)

SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$" )
