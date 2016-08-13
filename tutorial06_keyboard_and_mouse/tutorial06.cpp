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

struct v3 {
	float x, y, z;
};

class TexRecArray;

class TexRec {

	friend class TexRecArray;

	GLuint vertexbuffer;
	GLuint uvbuffer;

	std::vector<GLfloat> vertexes;
	std::vector<GLfloat> uvs;

	GLuint VertexArrayID;

	TextureID Texture;

public:
	TexRec(const std::string &TexturePath, v3 d, v3 c, v3 b, v3 a) : Texture(TextureManager.loadTexture(TexturePath)) {
		//glGenVertexArrays(1, &VertexArrayID);
		//glBindVertexArray(VertexArrayID);

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

		/*glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(GLfloat), vertexes.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(GLfloat), uvs.data(), GL_STATIC_DRAW); */
	}

	TexRec(const TexRec &Other) = delete;

	~TexRec() {
		/*glDeleteBuffers(1, &vertexbuffer);
		glDeleteBuffers(1, &uvbuffer);
		glDeleteVertexArrays(1, &VertexArrayID); */
	}

	void update() {
		//glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		//glBufferSubData(GL_ARRAY_BUFFER, 0, vertexes.size() * sizeof(GLfloat), vertexes.data());
	}

	void setHeights(float ah, float bh, float ch, float dh) {
		vertexes[1] = dh;
		vertexes[4] = ch;
		vertexes[7] = bh;
		vertexes[10] = bh;
		vertexes[13] = ah;
		vertexes[16] = dh;
		update();
	}

	const std::string& getTextureName() {
		return Texture.getPath();
	}

	void setHeights(float h) {
		for (int i = 0; i < 6; ++i) {
			vertexes[1 + i * 3] = h;
		}
		update();
	}

	void draw(float alpha) {
		/*Texture.activate();

		glUniform1f(1, alpha);

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
		glDisableVertexAttribArray(1); */
	}
};

class TexRecArray {

	GLuint vertexbuffer;
	GLuint uvbuffer;

	std::vector<GLfloat> vertexes;
	std::vector<GLfloat> uvs;

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
		glDeleteVertexArrays(1, &VertexArrayID);
	}

	void add(TexRec &Rec) {
		vertexes.insert(vertexes.begin(), Rec.vertexes.begin(), Rec.vertexes.end());
		uvs.insert(uvs.begin(), Rec.uvs.begin(), Rec.uvs.end());
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

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertexes.size() / 3);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}
};

class TexRecArrayMap {
	std::map<std::string, TexRecArray*> Arrays;
public:

	void add(TexRec &Rec) {
		auto I = Arrays.find(Rec.getTextureName());
		if (I == Arrays.end()) {
			Arrays[Rec.getTextureName()] = new TexRecArray(Rec.getTextureName());
			Arrays[Rec.getTextureName()]->add(Rec);
		} else {
			I->second->add(Rec);
		}
	}

	void finalize() {
		for (auto &Pair : Arrays) {
			Pair.second->finalize();
		}
	}

	void draw() {
		for (auto &Pair : Arrays) {
			Pair.second->draw();
		}
	}
};

class Tile {
	int Height[4] = {0, 0, 0, 0};
	int X = -1, Y = -1;
	bool Forest = false;
	bool City = false;
public:
	Tile() {
	}
	Tile(int X, int Y) : X(X), Y(Y) {
	}

	void makeForest() {
		Forest = true;
	}

	void makeCity() {
		City = true;
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

	bool isForest() const {
		if (isCity())
			return false;
		if (hasWater())
			return false;
		return Forest;
	}

	bool isCity() const {
		return City;
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

	bool isFullWater() const {
		for (int i = 0; i < 4; ++i)
			if (Height[i] >= 0)
				return false;
		return true;
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

	double percentageWater() {
		unsigned waterTiles = 0;
		for (int x = 0; x < Width; ++x) {
			for (int y = 0; y < Height; ++y) {
			  if (get(x, y).hasWater())
					++waterTiles;
			}
		}
		std::cout << (waterTiles / (double) (Width * Height)) << std::endl;
		return waterTiles / (double) (Width * Height);
	}

	void generate(int Seed) {
		noise::module::Perlin myModule;
		double offset = Seed;

		for (int x = 0; x < Width; ++x) {
			for (int y = 0; y < Height; ++y) {
				get(x, y) = Tile(x, y);
				double factor = 40.0;
				int height = (int) (myModule.GetValue(offset + x / factor, offset + y / factor, offset + 0.5) * 10);

				setHeight(x, y, height);
			}
		}
		offset = Seed + Seed + 1500;

		for (int x = 0; x < Width; ++x) {
			for (int y = 0; y < Height; ++y) {
				double factor = 20.0;
				double value = myModule.GetValue(offset - x / factor, offset - y / factor, offset + 0.9);

				if (value > 0.3)
					get(x, y).makeForest();
			}
		}
		offset = Seed + Seed + Seed + 3000;

		for (int x = 0; x < Width; ++x) {
			for (int y = 0; y < Height; ++y) {
				double factor = 40.0;
				double value = myModule.GetValue(offset - x / factor, offset - y / factor, offset + 0.9);

				if (value > 0.7)
					get(x, y).makeCity();
			}
		}
	}

public:
	TileMap(int Width, int Height, int Seed = 25) : Width(Width), Height(Height) {
		Tiles.resize(Width * Height);
		do {
			generate(Seed);
			Seed += 5;
		} while (percentageWater() > 0.60);
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

constexpr float TILE_SIZE = 0.1f;
constexpr float TILE_HEIGHT = 0.02f;


double getRandomFrac() {
	static std::random_device rd; // obtain a random number from hardware
	static std::mt19937 eng(rd()); // seed the generator
	static std::uniform_real_distribution<> distr(-1.0, 1.0); // define the range
	return distr(eng);
}

class Tree {
	std::vector<std::shared_ptr<TexRec> > Recs;

	float centerX, centerY;

	void makeHex(float HeightBottom, float HeightTop, float RadiusBottom, float RadiusTop) {
		std::vector<std::pair<float, float> > BottomCoords;
		std::vector<std::pair<float, float> > TopCoords;
		float staticRot = getRandomFrac()* (2 * M_PI) / 6;
		for (int i = 0; i <= 6; ++i) {
			float dx = std::cos(staticRot + i * (2 * M_PI) / 6);
			float dy = std::sin(staticRot + i * (2 * M_PI) / 6);
			BottomCoords.push_back(std::make_pair(centerX + dx * RadiusBottom, centerY + dy * RadiusBottom));
			TopCoords.push_back(std::make_pair(centerX + dx * RadiusTop, centerY + dy * RadiusTop));
		}

		for (int i = 1; i <= 6; ++i)
		  Recs.push_back(std::shared_ptr<TexRec>(new TexRec("forest.bmp",
				{BottomCoords[i - 1].first, HeightBottom, BottomCoords[i - 1].second},
																												{BottomCoords[i].first, HeightBottom, BottomCoords[i].second},
																												{TopCoords[i].first, HeightTop, TopCoords[i].second},
																												{TopCoords[i - 1].first, HeightTop, TopCoords[i - 1].second}
			)));
	}

public:
	Tree() {
	}
	Tree(int x, int y, int height) {
		centerX = TILE_SIZE * (x + 0.5f) + getRandomFrac() * TILE_SIZE / 4;
		centerY = TILE_SIZE * (y + 0.5f) + getRandomFrac() * TILE_SIZE / 4;
		float Scale = TILE_HEIGHT + getRandomFrac() * TILE_HEIGHT / 8;
		makeHex(height * Scale, (height + 2) * Scale, TILE_SIZE * 0.45f, TILE_SIZE * 0.2f);
		makeHex((height + 2) * Scale, (height + 4) * Scale, TILE_SIZE * 0.3f, TILE_SIZE * 0.1f);
		makeHex((height + 4) * Scale, (height + 6) * Scale, TILE_SIZE * 0.15f, 0);
	}

	void draw() {
		for (auto &Rec : Recs)
			Rec->draw(1.0f);
	}

	void addRecsTo(TexRecArrayMap& ArrayMap) {
		for (auto& Rec : Recs)
			ArrayMap.add(*Rec);
	}
};

class TileRenderer {

	std::shared_ptr<TexRec> Rec;

	bool IsWater = false;

	int x = 0;
	int y = 0;

	std::shared_ptr<Tree> t;

public:
	TileRenderer() {
	}
	TileRenderer(Tile &T) {
		float xOff = TILE_SIZE * T.getX();
		float yOff = TILE_SIZE * T.getY();
		std::string Texture = T.getY() != 13 ? "grass.bmp" : "street.bmp";
		if (T.hasWater())
			Texture = "sand.bmp";
		else if (T.isForest())
			Texture = "sand.bmp";
		else if (T.isCliff())
			Texture = "stones.bmp";
		else if (T.isCity())
			Texture = "city.bmp";

		Rec.reset(new TexRec(Texture,
												 {xOff       , TILE_HEIGHT * T.getHeight(0), TILE_SIZE + yOff},
												 {TILE_SIZE + xOff, TILE_HEIGHT * T.getHeight(1), TILE_SIZE + yOff},
												 {TILE_SIZE + xOff, TILE_HEIGHT * T.getHeight(2), yOff       },
												 {xOff       , TILE_HEIGHT * T.getHeight(3), yOff       }));

		if (T.isForest())
			t.reset(new Tree(T.getX(), T.getY(), T.getHeight(0)));
	}
	// for water
	TileRenderer(int x, int y) : x(x), y(y){
		float xOff = TILE_SIZE * x;
		float yOff = TILE_SIZE * y;
		Rec.reset(new TexRec("water.bmp",
												 {xOff       , TILE_HEIGHT * -0.5f, TILE_SIZE + yOff},
												 {TILE_SIZE + xOff, TILE_HEIGHT * -0.5f, TILE_SIZE + yOff},
												 {TILE_SIZE + xOff, TILE_HEIGHT * -0.5f, yOff       },
												 {xOff       , TILE_HEIGHT * -0.5f, yOff       }));
		IsWater = true;

	}

	void draw(double time) {
		if (IsWater) {
			float BaseZ = TILE_HEIGHT * -0.25f;
			float WaveHeight = TILE_HEIGHT * 0.13f;
			float WaveSpeed = 10;
			Rec->setHeights(BaseZ + WaveHeight * std::cos(time + x * WaveSpeed),
											BaseZ + WaveHeight * std::cos(time + (x + 1) * WaveSpeed),
											BaseZ + WaveHeight * std::cos(time + (x + 1) * WaveSpeed),
											BaseZ + WaveHeight * std::cos(time + x * WaveSpeed));
			Rec->draw(1.0f);
		}
	}

	void addRecsTo(TexRecArrayMap &ArrayMap) {
		ArrayMap.add(*Rec);
		if (t)
			t->addRecsTo(ArrayMap);
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

	//TileMap Map(110, 110, (int) (getRandomFrac() * 200000));
	TileMap Map(210, 210, 200000);

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
	// glEnable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("TransformVertexShader.vertexshader",
																 "TextureFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	{

		TexRecArrayMap StaticRecs;

		std::vector<TileRenderer> waterTiles;
		unsigned size = Map.getWidth() * Map.getHeight();
		unsigned i = 0;
		for (unsigned x = 0; x < Map.getWidth(); x++) {
			for (unsigned y = 0; y < Map.getHeight(); y++) {
				++i;
				if (i % 1024 == 0) {
					std::cout << "Created: " << (100.0 * i) / size << "%" << std::endl;
				}
				if (!Map.get(x, y).isFullWater()) {
					TileRenderer renderer(Map.get(x, y));
					renderer.addRecsTo(StaticRecs);
				}
				if (Map.get(x, y).hasWater())
					waterTiles.emplace_back(x, y);
			}
		}
		StaticRecs.finalize();

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


			namespace chr = std::chrono;

			chr::time_point<chr::steady_clock> tp = chr::steady_clock::now();

			double time = chr::duration_cast<chr::milliseconds>(tp.time_since_epoch()).count() / 1000.0;

			//for (TileRenderer &t : tiles)
			//	t.draw(time);
			StaticRecs.draw();

			for (TileRenderer &t : waterTiles)
				t.draw(time);

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

