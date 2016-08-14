// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include <noise/noise.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <vector>
#include <iostream>
#include <memory>
#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <math.h>
#include <map>
#include <cstring>
#include "Map.h"

class FPSCounter {
	unsigned frames = 0;
	std::chrono::steady_clock::time_point lastStart;

public:
	FPSCounter() {
		lastStart = std::chrono::steady_clock::now();
	}
	long getMillis() {
		auto diff = std::chrono::steady_clock::now() - lastStart;
		return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
	}
	void addFrame() {
		++frames;
		auto millis = getMillis();
		if (millis >= 1000) {
			lastStart = std::chrono::steady_clock::now();
			std::cout << (frames / (millis / 1000.0)) << std::endl;
			frames = 0;
		}
	}
};

# define M_PI           3.14159265358979323846  /* pi */

class Texture {

	GLuint Handle;

public:
	Texture() {
	}
	Texture(const std::string &Path) {
		Handle = loadBMP_custom(Path.c_str());
	}
	void activate() {
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Handle);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(Handle, 0);
	}
	void clear() {

	}
};

class TextureID {
	unsigned ID;
public:
	TextureID(unsigned ID) : ID(ID) {
	}
	void activate();

	const std::string &getPath();
};



class TextureManager {

	std::vector<Texture> Textures;
	std::unordered_map<std::string, unsigned> TexturesNamedToIDs;
	std::unordered_map<unsigned, std::string> IDsToTextureNames;

	unsigned LastActivatedID = std::numeric_limits<unsigned>::max();

public:
	TextureManager() {
	}

	TextureID loadTexture(const std::string &Path) {
		auto I = TexturesNamedToIDs.find(Path);
		if (I != TexturesNamedToIDs.end()) {
			return TextureID(I->second);
		} else {
			unsigned OldSize = Textures.size();
			Textures.push_back(Texture(Path));
			TexturesNamedToIDs[Path] = OldSize;
			IDsToTextureNames[OldSize] = Path;
			return TextureID(OldSize);
		}
	}

	Texture& operator[](unsigned ID) {
		return Textures[ID];
	}

	const std::string &getName(unsigned ID) {
		return IDsToTextureNames[ID];
	}

	void activate(unsigned ID) {
		if (ID != LastActivatedID) {
			operator[](ID).activate();
			LastActivatedID = ID;
		}
	}
};

TextureManager TextureManager;

void TextureID::activate() {
	TextureManager[ID].activate();
}

const std::string& TextureID::getPath() {
	return TextureManager.getName(ID);
}

struct v3f {
	GLfloat x, y, z;
	v3f(GLfloat x, GLfloat y, GLfloat z) : x(x), y(y), z(z) {

	}
};

struct Rec {

	std::vector<GLfloat> vertexes;
	std::vector<GLfloat> uvs;

	Rec(const v3f &d, const v3f &c, const v3f &b, const v3f &a,
			std::pair<float, float> uvStart, float us, float vs) {

		std::pair<float, float> uvEnd = std::make_pair(uvStart.first + us,
		                                               uvStart.second + vs);
		vertexes = {
			d.x, d.y, d.z,
			c.x, c.y, c.z,
			b.x, b.y, b.z,
			b.x, b.y, b.z,
			a.x, a.y, a.z,
			d.x, d.y, d.z,
		};

		// Two UV coordinatesfor each vertex. They were created withe Blender.
		uvs = {
			uvStart.first, uvStart.second,
			uvEnd.first,   uvStart.second,
			uvEnd.first,   uvEnd.second,
			uvEnd.first,   uvEnd.second,
			uvStart.first, uvEnd.second,
			uvStart.first, uvStart.second,
		};
	}

};

class TexRecArray {

	GLuint vertexbuffer;
	GLuint uvbuffer;
	GLuint lightbuffer;

	std::vector<GLfloat> vertexes;
	std::vector<GLfloat> uvs;
	std::vector<GLfloat> lights;

	GLuint VertexArrayID;

	TextureID Texture;

	bool Finalized = false;

public:
	TexRecArray(const std::string &TexturePath) : Texture(TextureManager.loadTexture(TexturePath)) {
	}

	~TexRecArray() {
		if (!Finalized)
			return;
		glDeleteBuffers(1, &vertexbuffer);
		glDeleteBuffers(1, &uvbuffer);
		glDeleteBuffers(1, &lightbuffer);
		glDeleteVertexArrays(1, &VertexArrayID);
	}

	void add(const std::vector<GLfloat> &v, const std::vector<GLfloat> &u, float light) {
		vertexes.resize(vertexes.size() + 18);
		std::memcpy(vertexes.data() + vertexes.size() - 18, v.data(), sizeof(GLfloat) * 18);
		uvs.resize(uvs.size() + 12);
		std::memcpy(uvs.data() + uvs.size() - 12, u.data(), sizeof(GLfloat) * 12);
		lights.push_back(light);
		lights.push_back(light);
		lights.push_back(light);
		lights.push_back(light);
		lights.push_back(light);
		lights.push_back(light);

		//vertexes.insert(vertexes.begin(), v.begin(), v.end());
		//uvs.insert(uvs.begin(), u.begin(), u.end());
	}

	void add(const Rec &R) {
		vertexes.insert(vertexes.begin(), R.vertexes.begin(), R.vertexes.end());
		uvs.insert(uvs.begin(), R.uvs.begin(), R.uvs.end());
	}

	void finalize() {
		assert(!Finalized);
		Finalized = true;

		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(GLfloat), vertexes.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(GLfloat), uvs.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &lightbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, lightbuffer);
		glBufferData(GL_ARRAY_BUFFER, lights.size() * sizeof(GLfloat), lights.data(), GL_STATIC_DRAW);
	}

	void draw() {
		Texture.activate();

		glUniform1f(1, 1.0f);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3nd attribute buffer : Lights
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, lightbuffer);
		glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			1,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);


		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertexes.size() / 3);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
	}
};


#define ADD_VOXEL_SIDE(Ax, Ay, Az, Bx, By, Bz, Cx, Cy, Cz, Dx, Dy, Dz, side) \
  if (V.S[side].isFree()){                                         \
    GLfloat u = V.getUVOffset(side).first;                         \
    GLfloat v = V.getUVOffset(side).second;                        \
    Array.add({   (float) Ax, (float) Ay, (float) Az,              \
                  (float) Bx, (float) By, (float) Bz,              \
                  (float) Cx, (float) Cy, (float) Cz,              \
                  (float) Cx, (float) Cy, (float) Cz,              \
                  (float) Dx, (float) Dy, (float) Dz,              \
                  (float) Ax, (float) Ay, (float) Az}, {           \
                  u, v,             															 \
                  u + us,   v,                                     \
                  u + us,   v + vs,                                \
                  u + us,   v + vs,                                \
                  u, v + vs,                                       \
                  u, v                                             \
                  }, V.S[side].lightPercent());                    \
  }

class VoxelMapRenderer {

	VoxelChunk &Map;
	TexRecArray Array;

public:
	VoxelMapRenderer(VoxelChunk &Map) : Map(Map), Array("textures.bmp") {
		recreate();
	}

	void recreate() {
		for (int64_t x = 0; x < Map.getSize().x; ++x) {
			for (int64_t y = 0; y < Map.getSize().y; ++y) {
				for (int64_t z = 0; z < Map.getSize().z; ++z) {
					AnnotatedVoxel V = Map.getAnnotated({x, y, z});
					if (V.V.isFree())
						continue;
					const float us = Voxel::TEX_SIZE;
					const float vs = us;

					ADD_VOXEL_SIDE(
						x,     y + 1, z,
						x,     y + 1, z + 1,
						x + 1, y + 1, z + 1,
						x + 1, y + 1, z,
						0);
					ADD_VOXEL_SIDE(
						x,     y, z,
						x,     y, z + 1,
						x + 1, y, z + 1,
						x + 1, y, z,
						1);
					ADD_VOXEL_SIDE(
						x,     y + 1, z,
						x + 1, y + 1, z,
						x + 1, y, z,
						x,     y, z,
						2);
					ADD_VOXEL_SIDE(
						x,     y + 1, z + 1,
						x + 1, y + 1, z + 1,
						x + 1, y, z + 1,
						x,     y, z + 1,
						3);
					ADD_VOXEL_SIDE(
						x, y + 1, z + 1,
						x, y + 1, z,
						x,     y, z,
						x,     y, z + 1,
						4);
					ADD_VOXEL_SIDE(
						x + 1, y + 1, z + 1,
						x + 1, y + 1, z,
						x + 1,     y, z,
						x + 1,     y, z + 1,
						5);
				}
			}
		}
		Array.finalize();
	}

	void draw() {
		Array.draw();
	}

};



int main( void ) {
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	FPSCounter Counter;

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
								 GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Tutorial 0 - Keyboard and Mouse", NULL,
														NULL);
	if (window == NULL) {
		fprintf(stderr,
						"Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	// glEnable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("TransformVertexShader.vertexshader",
																 "TextureFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	glfwSwapInterval(0);

	VoxelChunk map({200, 200, 200});
	std::cout << "Created\n";
	FPSCounter timer;
	VoxelMapRenderer mapRenderer(map);
	std::cout << "Made chunk in " << timer.getMillis() << " millis\n";

	{

		do {

			// Clear the screen
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Use our shader
			glUseProgram(programID);

			// Compute the MVP matrix from keyboard and mouse input
			computeMatricesFromInputs();
			glm::mat4 ProjectionMatrix = getProjectionMatrix();
			glm::mat4 ViewMatrix = getViewMatrix();
			glm::mat4 ModelMatrix = glm::mat4(1.0);
			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

			// Send our transformation to the currently bound shader,
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);


			mapRenderer.draw();

			// Swap buffers
			glfwSwapBuffers(window);
			glfwPollEvents();

			Counter.addFrame();

		} // Check if the ESC key was pressed or the window was closed
		while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
					 glfwWindowShouldClose(window) == 0);

	}

	// Cleanup VBO and shader
	glDeleteProgram(programID);
	glDeleteTextures(1, &TextureID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

