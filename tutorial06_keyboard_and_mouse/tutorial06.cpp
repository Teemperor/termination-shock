// Include standard headers
#include <stdio.h>
#include <stdlib.h>

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
};


class TextureManager {

	std::vector<Texture> Textures;
	std::unordered_map<std::string, unsigned> TexturesNamedToIDs;

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
			return TextureID(OldSize);
		}
	}

	Texture& operator[](unsigned ID) {
		return Textures[ID];
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

struct v3 {
	float x, y, z;
};

class TexRec {

	GLuint vertexbuffer;
	GLuint uvbuffer;

	std::vector<GLfloat> vertexes;
	std::vector<GLfloat> uvs;

	GLuint VertexArrayID;

	TextureID Texture;

public:
	TexRec(const std::string &TexturePath, v3 a, v3 b, v3 c, v3 d) : Texture(TextureManager.loadTexture(TexturePath)) {
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		// Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
		// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
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
			0, 0,
			1, 0,
			1, 1,
			1, 1,
			0, 1,
			0, 0,
		};

		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(GLfloat), vertexes.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(GLfloat), uvs.data(), GL_STATIC_DRAW);
	}

	TexRec(const TexRec &Other) = delete;

	~TexRec() {
		glDeleteBuffers(1, &vertexbuffer);
		glDeleteBuffers(1, &uvbuffer);
		glDeleteVertexArrays(1, &VertexArrayID);
	}

	void update() {
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertexes.size() * sizeof(GLfloat), vertexes.data());
	}

	void raise() {
		vertexes[1] += 0.01f;
		update();
	}

	void draw() {
		Texture.activate();

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

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 6); // 12*3 indices starting at 0 -> 12 triangles

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}
};

#define SIZE 0.1f

class Tile {

	std::shared_ptr<TexRec> Rec;

public:
	Tile() {
	}
	Tile(int x, int y) {
		float xOff = SIZE * x;
		float yOff = SIZE * y;
		Rec.reset(new TexRec(y != 8 ? "grass.bmp" : "street.bmp",
												 {xOff       , 0, SIZE + yOff},
												 {SIZE + xOff, 0, SIZE + yOff},
												 {SIZE + xOff, 0, yOff       },
												 {xOff       , 0, yOff       }));
	}

	void draw() {
		Rec->draw();
	}

};

class FPSCounter {
	unsigned frames = 0;
	std::chrono::steady_clock::time_point lastStart;
public:
	FPSCounter() {
		lastStart = std::chrono::steady_clock::now();
	}
	void addFrame() {
		++frames;
		auto diff = std::chrono::steady_clock::now() - lastStart;
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
		if (millis >= 1000) {
			lastStart = std::chrono::steady_clock::now();
			std::cout << (frames / (millis / 1000.0)) << std::endl;
			frames = 0;
		}
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
	glClearColor(0.0f, 0.0f, 0.1f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("TransformVertexShader.vertexshader",
																 "TextureFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	{

		std::vector<Tile> tiles;
		for (unsigned x = 0; x < 210; x++) {
			for (unsigned y = 0; y < 210; y++) {
				tiles.emplace_back(x, y);
			}
		}

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

			for (Tile &t : tiles)
				t.draw();

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

