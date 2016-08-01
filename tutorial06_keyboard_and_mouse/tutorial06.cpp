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

	void setHeights(float h) {
		for (int i = 0; i < 6; ++i) {
			vertexes[1 + i * 3] = h;
		}
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
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}
};

class Tile {
	int Height[4] = {0, 0, 0, 0};
	int X = -1, Y = -1;
public:
	Tile() {
	}
	Tile(int X, int Y) : X(X), Y(Y) {
	}

	int getHeight(int i) const {
		return Height[i];
	}

	void setHeight(int i, int value) {
		Height[i] = value;
	}

	int getX() const {
		return X;
	}

	int getY() const {
		return Y;
	}

	operator bool() const {
		return X >= 0;
	}

	bool hasWater() const {
		for (int i = 0; i < 4; ++i)
			if (Height[i] < 0)
				return true;
		return false;
	}

	bool isCliff() const {
		auto max = std::max_element(std::begin(Height), std::end(Height));
		auto min = std::min_element(std::begin(Height), std::end(Height));
		return (*max - *min) >= 4;
	}

};

class TileMap {
	std::vector<Tile> Tiles;
	int Width;
	int Height;

	Tile DefaultTile;

	void generate() {
		noise::module::Perlin myModule;
		double offset = std::random_device()();
		std::cerr << offset << std::endl;

		for (int x = 0; x < Width; ++x) {
			for (int y = 0; y < Width; ++y) {
				get(x, y) = Tile(x, y);
				double factor = 40.0;
				int height = (int) (myModule.GetValue(offset + x / factor, offset + y / factor, offset + 0.5) * 10);

				setHeight(x, y, height);
			}
		}
	}

public:
	TileMap(int Width, int Height) : Width(Width), Height(Height) {
		Tiles.resize(Width * Height);
		generate();
	}

	Tile& get(int X, int Y) {
		if (X < 0 || Y < 0 || X >= Width)
			return DefaultTile;
		size_t Index = X + Y * Width;
		if (Index >= Tiles.size())
			return DefaultTile;
		return Tiles[Index];
	}

	void setHeight(int X, int Y, int Height) {
		for(int i = 0; i < 4; ++i)
			get(X, Y).setHeight(i, Height);
		get(X + 1, Y + 1).setHeight(3, Height);
		get(X - 1, Y + 1).setHeight(2, Height);
		get(X - 1, Y - 1).setHeight(1, Height);
		get(X + 1, Y - 1).setHeight(0, Height);

		get(X + 1, Y).setHeight(0, Height);
		get(X + 1, Y).setHeight(3, Height);

		get(X - 1, Y).setHeight(1, Height);
		get(X - 1, Y).setHeight(2, Height);

		get(X, Y - 1).setHeight(0, Height);
		get(X, Y - 1).setHeight(1, Height);

		get(X, Y + 1).setHeight(2, Height);
		get(X, Y + 1).setHeight(3, Height);
	}

	int getWidth() {
		return Width;
	}

	int getHeight() {
		return Height;
	}

};

class TileRenderer {

	std::shared_ptr<TexRec> Rec;

	static constexpr float SIZE = 0.1f;
	static constexpr float HEIGHT = 0.02f;

	bool IsWater = false;

	int x = 0;
	int y = 0;

public:
	TileRenderer() {
	}
	TileRenderer(Tile &T) {
		float xOff = SIZE * T.getX();
		float yOff = SIZE * T.getY();
		std::string Texture = T.getY() != 13 ? "grass.bmp" : "street.bmp";
		if (T.isCliff() || T.hasWater())
			Texture = "stones.bmp";
		Rec.reset(new TexRec(Texture,
												 {xOff       , HEIGHT * T.getHeight(0), SIZE + yOff},
												 {SIZE + xOff, HEIGHT * T.getHeight(1), SIZE + yOff},
												 {SIZE + xOff, HEIGHT * T.getHeight(2), yOff       },
												 {xOff       , HEIGHT * T.getHeight(3), yOff       }));
	}
	// for water
	TileRenderer(int x, int y) : x(x), y(y){
		float xOff = SIZE * x;
		float yOff = SIZE * y;
		Rec.reset(new TexRec("water.bmp",
												 {xOff       , HEIGHT * -0.5f, SIZE + yOff},
												 {SIZE + xOff, HEIGHT * -0.5f, SIZE + yOff},
												 {SIZE + xOff, HEIGHT * -0.5f, yOff       },
												 {xOff       , HEIGHT * -0.5f, yOff       }));
		IsWater = true;

	}

	void draw() {
		if (IsWater) {
			namespace chr = std::chrono;

			chr::time_point<chr::steady_clock> tp = chr::steady_clock::now();

			float waterDwindle = std::cos(chr::duration_cast<chr::milliseconds>(tp.time_since_epoch()).count() / 1000.0);
			Rec->setHeights(HEIGHT * -0.25f + HEIGHT * 0.13f * waterDwindle);
		}
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

	TileMap Map(100, 100);
	/*for (unsigned x = 0; x < Map.getWidth(); x++) {
		for (unsigned y = 0; y < Map.getHeight(); y++) {
			Map.get(x, y) = Tile(x, y);
		}
	}


	Map.setHeight(8, 8, 1);
	Map.setHeight(8, 9, 1);
	Map.setHeight(9, 9, 2);
	Map.setHeight(19, 19, 3);
	Map.setHeight(11, 11, -1);

	Map.setHeight(89, 89, 6);
	Map.setHeight(89, 87, 6);
	Map.setHeight(89, 88, 6);
	Map.setHeight(88, 89, 6);
	Map.setHeight(81, 81, -1);

	for (int x = 44; x < 66; x++) {
		for (int y = 22; y < 33; y++) {

			Map.setHeight(x, y, -1);
		}
	}
*/
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
	glEnable( GL_BLEND );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

		std::vector<TileRenderer> tiles;
		for (unsigned x = 0; x < Map.getWidth(); x++) {
			for (unsigned y = 0; y < Map.getHeight(); y++) {
				tiles.emplace_back(Map.get(x, y));
				if (Map.get(x, y).hasWater())
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

			for (TileRenderer &t : tiles)
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

