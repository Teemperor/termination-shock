// Include standard headers
#include <stdio.h>
#include <stdlib.h>


#include "RenderWindow.h"

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <vector>
#include <memory>
#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <map>
#include <cstring>
#include <sstream>
#include "Map.h"
#include "Camera.h"
#include "Controls.h"

# define M_PI           3.14159265358979323846  /* pi */

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


using namespace std;

void split(const string &s, char delim, vector<string> &elems) {
  stringstream ss(s);
  string item;
  while (getline(ss, item, delim)) {
    elems.push_back(item);
  }
}


vector<string> split(const string &s, char delim) {
  vector<string> elems;
  split(s, delim, elems);
  return elems;
}

class Texture {

  GLuint Handle;

public:
  Texture() {
  }

  Texture(const std::string &Path) {
    auto Parts = split(Path, ':');
    assert(Parts.size() == 2);
    assert(Parts[1] == "linear" || Parts[1] == "nearest");
    Handle = loadBMP_custom(Parts[0].c_str(), Parts[1] == "nearest");
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

  Texture &operator[](unsigned ID) {
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

const std::string &TextureID::getPath() {
  return TextureManager.getName(ID);
}

struct Rec {

  std::vector<GLfloat> vertexes;
  std::vector<GLfloat> uvs;

  Rec(const v3f &d, const v3f &c, const v3f &b, const v3f &a,
      std::pair<float, float> uvStart = {0, 0}, float us = 1, float vs = 1) {

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
      uvEnd.first, uvStart.second,
      uvEnd.first, uvEnd.second,
      uvEnd.first, uvEnd.second,
      uvStart.first, uvEnd.second,
      uvStart.first, uvStart.second,
    };
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
  TexRecArray(const std::string &TexturePath) : Texture(
    TextureManager.loadTexture(TexturePath)) {
  }

  ~TexRecArray() {
    reset();
  }

  void add(const Rec &R) {
    vertexes.insert(vertexes.begin(), R.vertexes.begin(), R.vertexes.end());
    uvs.insert(uvs.begin(), R.uvs.begin(), R.uvs.end());
  }

  void reset() {
    vertexes.clear();
    uvs.clear();
    if (!Finalized)
      return;
    Finalized = false;
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
  }

  void finalize() {
    assert(!Finalized);
    Finalized = true;

    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(GLfloat),
                 vertexes.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(GLfloat), uvs.data(),
                 GL_STATIC_DRAW);
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
      (void *) 0          // array buffer offset
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
      (void *) 0                        // array buffer offset
    );

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, vertexes.size() / 3);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
  }
};


class BlockSideArray {

  GLuint vertexbuffer;
  GLuint uvbuffer;
  GLuint lightbuffer;
  GLuint occlusionbuffer;

  std::vector<GLfloat> vertexes;
  std::vector<GLfloat> uvs;
  std::vector<GLfloat> lights;
  std::vector<GLfloat> occlusion;

  GLuint VertexArrayID;

  TextureID Texture;

  bool Finalized = false;

public:
  BlockSideArray(const std::string &TexturePath) : Texture(
    TextureManager.loadTexture(TexturePath)) {
  }

  ~BlockSideArray() {
    reset();
  }

  void add(const std::vector<GLfloat> &v, const std::vector<GLfloat> &u,
           float light, const std::array<GLfloat, 6> &o) {
    vertexes.resize(vertexes.size() + 18);
    std::memcpy(vertexes.data() + vertexes.size() - 18, v.data(),
                sizeof(GLfloat) * 18);
    uvs.resize(uvs.size() + 12);
    std::memcpy(uvs.data() + uvs.size() - 12, u.data(), sizeof(GLfloat) * 12);
    lights.push_back(light);
    lights.push_back(light);
    lights.push_back(light);
    lights.push_back(light);
    lights.push_back(light);
    lights.push_back(light);

    occlusion.resize(occlusion.size() + o.size());
    std::memcpy(occlusion.data() + occlusion.size() - o.size(), o.data(), sizeof(GLfloat) * o.size());

    //vertexes.insert(vertexes.begin(), v.begin(), v.end());
    //uvs.insert(uvs.begin(), u.begin(), u.end());
  }

  void add(const Rec &R) {
    vertexes.insert(vertexes.begin(), R.vertexes.begin(), R.vertexes.end());
    uvs.insert(uvs.begin(), R.uvs.begin(), R.uvs.end());
  }

  void reset() {
    vertexes.clear();
    uvs.clear();
    lights.clear();
    occlusion.clear();
    if (!Finalized)
      return;
    Finalized = false;
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &lightbuffer);
    glDeleteBuffers(1, &occlusionbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
  }

  void finalize() {
    assert(!Finalized);
    Finalized = true;

    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(GLfloat),
                 vertexes.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(GLfloat), uvs.data(),
                 GL_STATIC_DRAW);

    glGenBuffers(1, &lightbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, lightbuffer);
    glBufferData(GL_ARRAY_BUFFER, lights.size() * sizeof(GLfloat),
                 lights.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &occlusionbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, occlusionbuffer);
    glBufferData(GL_ARRAY_BUFFER, occlusion.size() * sizeof(GLfloat),
                 occlusion.data(), GL_STATIC_DRAW);
  }

  void draw() {
    if (vertexes.empty())
      return;

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
      (void *) 0          // array buffer offset
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
      (void *) 0                        // array buffer offset
    );

    // 3nd attribute buffer : Lights
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, lightbuffer);
    glVertexAttribPointer(
      2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
      1,                                // size : one float
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void *) 0                        // array buffer offset
    );

    // 4nd attribute buffer : Occlussion lightning
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, occlusionbuffer);
    glVertexAttribPointer(
      3,                                // attribute. No particular reason for 1, but must match the layout in the shader.
      1,                                // size : one float
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void *) 0                        // array buffer offset
    );


    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, vertexes.size() / 3);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
  }
};

class DeepSpaceRenderer {
  BlockSideArray Array;

public:
  DeepSpaceRenderer() : Array("earth.bmp:linear") {
    const float size = 15000;
    const float distance = -19000;
    Array.add(Rec(v3f(size, -size, distance), v3f(size, size, distance), v3f(-size, size, distance), v3f(-size, -size, distance)));
    Array.finalize();
  }

  void draw() {
    Array.draw();
  }
};


#define ADD_VOXEL_SIDE(Ax, Ay, Az, Bx, By, Bz, Cx, Cy, Cz, Dx, Dy, Dz, side) \
  if (!V.S[side].blocksView()){                                    \
    GLfloat u = V.getUVOffset(side).first;                         \
    GLfloat v = V.getUVOffset(side).second;                        \
    Array.add({   (float) Ax, (float) Ay, (float) Az,              \
                  (float) Bx, (float) By, (float) Bz,              \
                  (float) Cx, (float) Cy, (float) Cz,              \
                  (float) Cx, (float) Cy, (float) Cz,              \
                  (float) Dx, (float) Dy, (float) Dz,              \
                  (float) Ax, (float) Ay, (float) Az}, {           \
                  u, v,                                            \
                  u + us,   v,                                     \
                  u + us,   v + vs,                                \
                  u + us,   v + vs,                                \
                  u, v + vs,                                       \
                  u, v                                             \
                  }, V.S[side].lightPercent(),                     \
                  getOcclusionLighting({x, y, z}, side));          \
  }

class VoxelMapRenderer {

  VoxelChunk *Map;
  BlockSideArray Array;

  v3 offset;
  v3 size = v3(getSize(), getSize(), getSize());

  static constexpr float ONE_THIRD = 1.0f / 3.0f;

#define LIGHT_SUM(ax, ay, az, bx, by, bz, cx, cy, cz) \
   (Map->get({pos.x + ax, pos.y + ay, pos.z + az}).isFree() ? ONE_THIRD : 0.1f) \
 + (Map->get({pos.x + bx, pos.y + by, pos.z + bz}).isFree() ? ONE_THIRD : 0.1f) \
 + (Map->get({pos.x + cx, pos.y + cy, pos.z + cz}).isFree() ? ONE_THIRD : 0.1f);

  std::array<GLfloat, 6> getOcclusionLighting(const v3& pos, unsigned side) {
    std::array<GLfloat, 6> Result = {1, 1, 1, 1, 1, 1};
    switch(side) {
      case 0: {
        Result[2] = Result[3] =  LIGHT_SUM( 1,  1,  0,
                                            1,  1,  1,
                                            0,  1,  1);
        Result[0] = Result[5] =  LIGHT_SUM(-1,  1,  0,
                                           -1,  1, -1,
                                            0,  1, -1);
        Result[1] =              LIGHT_SUM(-1,  1,  0,
                                           -1,  1,  1,
                                            0,  1,  1);
        Result[4] =              LIGHT_SUM( 1,  1,  0,
                                            1,  1, -1,
                                            0,  1, -1);
        break;
      }
      case 1: {
        Result[2] = Result[3] =  LIGHT_SUM( 1, -1,  0,
                                            1, -1,  1,
                                            0, -1,  1);
        Result[0] = Result[5] =  LIGHT_SUM(-1, -1,  0,
                                           -1, -1, -1,
                                            0, -1, -1);
        Result[1] =              LIGHT_SUM(-1, -1,  0,
                                           -1, -1,  1,
                                            0, -1,  1);
        Result[4] =              LIGHT_SUM( 1, -1,  0,
                                            1, -1, -1,
                                            0, -1, -1);
        break;
      }
      case 2: {
        Result[2] = Result[3] =  LIGHT_SUM( 1,  0, -1,
                                            0, -1, -1,
                                            1, -1, -1);
        Result[0] = Result[5] =  LIGHT_SUM(-1,  0, -1,
                                            0,  1, -1,
                                           -1,  1, -1);
        Result[1] =              LIGHT_SUM( 1,  0, -1,
                                            0,  1, -1,
                                            1,  1, -1);
        Result[4] =              LIGHT_SUM(-1,  0, -1,
                                            0, -1, -1,
                                           -1, -1, -1);
        break;
      }
      case 3: {
        Result[2] = Result[3] =  LIGHT_SUM( 1,  0,  1,
                                            0, -1,  1,
                                            1, -1,  1);
        Result[0] = Result[5] =  LIGHT_SUM(-1,  0,  1,
                                            0,  1,  1,
                                           -1,  1,  1);
        Result[1] =              LIGHT_SUM( 1,  0,  1,
                                            0,  1,  1,
                                            1,  1,  1);
        Result[4] =              LIGHT_SUM(-1,  0,  1,
                                            0, -1,  1,
                                           -1, -1,  1);
        break;
      }
      case 4: {
        Result[2] = Result[3] =  LIGHT_SUM(-1, -1, -1,
                                           -1,  0, -1,
                                           -1, -1,  0);
        Result[0] = Result[5] =  LIGHT_SUM(-1,  1,  1,
                                           -1,  0,  1,
                                           -1,  1,  0);
        Result[1] =              LIGHT_SUM(-1,  1, -1,
                                           -1,  0, -1,
                                           -1,  1,  0);
        Result[4] =              LIGHT_SUM(-1, -1,  1,
                                           -1,  0,  1,
                                           -1, -1,  0);
        break;
      }
      case 5: {
        Result[2] = Result[3] =  LIGHT_SUM( 1, -1, -1,
                                            1,  0, -1,
                                            1, -1,  0);
        Result[0] = Result[5] =  LIGHT_SUM( 1,  1,  1,
                                            1,  0,  1,
                                            1,  1,  0);
        Result[1] =              LIGHT_SUM( 1,  1, -1,
                                            1,  0, -1,
                                            1,  1,  0);
        Result[4] =              LIGHT_SUM( 1, -1,  1,
                                            1,  0,  1,
                                            1, -1,  0);
        break;
      }
    }
    return Result;
  }

public:
  VoxelMapRenderer(VoxelChunk &Map, v3 offset) : Array("textures.bmp:nearest") {
    setMap(&Map, offset);
  }
  VoxelMapRenderer() : Array("textures.bmp:nearest") {
  }

  static size_t getSize() {
    return 16;
  }

  void setMap(VoxelChunk *M, v3 offset) {
    this->offset = offset;
    this->size = size;
    Map = M;
    recreate();
  }

  void recreate() {
    Array.reset();
    for (int64_t x = offset.x; x < size.x + offset.x; ++x) {
      for (int64_t y = offset.y; y < size.y + offset.y; ++y) {
        for (int64_t z = offset.z; z < size.z + offset.z; ++z) {
          AnnotatedVoxel V = Map->getAnnotated(v3(x, y, z));
          if (V.V.isFree())
            continue;
          const float us = Voxel::TEX_SIZE;
          const float vs = us;

          ADD_VOXEL_SIDE(
            x, y + 1, z,
            x, y + 1, z + 1,
            x + 1, y + 1, z + 1,
            x + 1, y + 1, z,
            0);
          ADD_VOXEL_SIDE(
            x, y, z,
            x, y, z + 1,
            x + 1, y, z + 1,
            x + 1, y, z,
            1);
          ADD_VOXEL_SIDE(
            x, y + 1, z,
            x + 1, y + 1, z,
            x + 1, y, z,
            x, y, z,
            2);
          ADD_VOXEL_SIDE(
            x, y + 1, z + 1,
            x + 1, y + 1, z + 1,
            x + 1, y, z + 1,
            x, y, z + 1,
            3);
          ADD_VOXEL_SIDE(
            x, y + 1, z + 1,
            x, y + 1, z,
            x, y, z,
            x, y, z + 1,
            4);
          ADD_VOXEL_SIDE(
            x + 1, y + 1, z + 1,
            x + 1, y + 1, z,
            x + 1, y, z,
            x + 1, y, z + 1,
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

class VoxelRenderMap {

  std::vector<VoxelMapRenderer *> Renders;

public:
  VoxelRenderMap(VoxelChunk& Chunk) {
    const size_t renderSize = VoxelMapRenderer::getSize();
    for (int64_t x = 0; x < Chunk.getSize(); x += renderSize)
      for (int64_t y = 0; y < Chunk.getSize(); y += renderSize)
        for (int64_t z = 0; z < Chunk.getSize(); z += renderSize)
          Renders.push_back(new VoxelMapRenderer(Chunk, v3(x, y, z)));
  }

  VoxelMapRenderer *get(v3 pos) {
    size_t rs = VoxelChunk::getSize() / VoxelMapRenderer::getSize();

    size_t index = (pos.x / VoxelMapRenderer::getSize()) * rs * rs + (pos.y / VoxelMapRenderer::getSize()) * rs + (pos.z / VoxelMapRenderer::getSize());
    if (index < Renders.size())
      return Renders[index];
    return nullptr;
  }

  void recreateSurrounding(v3 pos) {
    static const std::array<v3, 7> Offsets = {
      v3(0, 0, 0),
      v3(0, 0, 1),
      v3(0, 0, -1),
      v3(0, 1, 0),
      v3(0, -1, 0),
      v3(1, 0, 0),
      v3(-1, 0, 0)
    };

    for (int x = -1; x <= 1; ++x)
      for (int y = -1; y <= 1; ++y)
        for (int z = -1; z <= 1; ++z)
          if (auto R = get(v3(pos.x + x * VoxelMapRenderer::getSize(),
                           pos.y + y * VoxelMapRenderer::getSize(),
                           pos.z + z * VoxelMapRenderer::getSize())))
            R->recreate();
  }

  void draw() {
    for (auto &R : Renders)
      R->draw();
  }
};

int main(int argc, char** argv) {

  Camera camera;
  Controls controls;
  RenderWindow window(1024, 768);


  // Dark blue background
  glClearColor(0.00f, 0.00f, 0.00f, 1.0f);

  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Cull triangles which normal is not towards the camera
  // glEnable(GL_CULL_FACE);

  // Create and compile our GLSL program from the shaders
  GLuint BlockProgramID = LoadShaders("BlockVertexShader.vert",
                                 "BlockFragmentShader.frag");

  // Get a handle for our "MVP" uniform
  GLuint MatrixID = glGetUniformLocation(BlockProgramID, "MVP");

  // Get a handle for our "myTextureSampler" uniform
  GLuint TextureID = glGetUniformLocation(BlockProgramID, "myTextureSampler");


  // Create and compile our GLSL program from the shaders
  GLuint SpaceProgramID = LoadShaders("DeepSpaceVertexShader.vert",
                                      "DeepSpaceFragmentShader.frag");

  // Get a handle for our "MVP" uniform
  GLuint SpaceMatrixID = glGetUniformLocation(SpaceProgramID, "MVP");

  // Get a handle for our "myTextureSampler" uniform
  GLuint SpaceTextureID = glGetUniformLocation(SpaceProgramID, "myTextureSampler");


  FPSCounter Counter;


  bool run = true;

  //for (int x = 0; x < 100; x += 32) {
  //  for (int z = 0; z < 100; z += 32) {
  //    Chunks.push_back(VoxelChunk({x, 0, z}));
  ///  }
  //}

  VoxelChunk Chunk({0, 0, 0});

  VoxelRenderMap Renderer(Chunk);

  DeepSpaceRenderer DeepSpace;

  MovingLight CameraLight(&Chunk);

  MovingEntity Player(&Chunk);

  std::vector<Voxel::Types> BlockTypes = {
    Voxel::CRATE,
    Voxel::STEEL_FLOOR,
    Voxel::STEEL_WALL,
    Voxel::GENERATOR
  };
  Voxel::Types SelectedType = BlockTypes[0];

  Uint64 LAST = 0;

  do {

    Uint64 NOW = SDL_GetPerformanceCounter();
    float deltaTime = (((float) NOW - LAST) / SDL_GetPerformanceFrequency());
    if (LAST == 0)
      deltaTime = 0;
    LAST = NOW;

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // Compute the MVP matrix from keyboard and mouse input
    glm::mat4 ProjectionMatrix = camera.getProjectionMatrix();
    glm::mat4 ViewMatrix = camera.getViewMatrix();
    glm::mat4 ModelMatrix = glm::mat4(1.0);
    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

    // Use our shader
    glUseProgram(BlockProgramID);

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    Renderer.draw();

    // Use our shader
    glUseProgram(SpaceProgramID);

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(SpaceMatrixID, 1, GL_FALSE, &MVP[0][0]);

    DeepSpace.draw();

    Counter.addFrame();

    window.swap();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
        run = false;
      controls.handleEvent(event);
      camera.handleEvent(event);
    }
    controls.update();
    //camera.updatePos(controls.getX(), controls.getY());

    if (controls.jumpPoll())
      if (Player.onGround())
        Player.jump();
    Player.setMove(camera.getHorizAngle(), controls.getX(), controls.getY());
    Player.update(deltaTime);
    camera.setPos(Player.position().x, Player.position().y, Player.position().z);

    Chunk.update(deltaTime);

    std::cout << "Current V( " << Player.position().toVoxelPos() << "): " << Chunk.get(Player.position().toVoxelPos()).getName() << std::endl;

    if (controls.getBlockType() < BlockTypes.size())
      SelectedType = BlockTypes[controls.getBlockType()];

    if (false) {
      auto cameraPos = camera.getPosition();
      v3 cameraVoxel((int64_t) cameraPos.x, (int64_t) cameraPos.y,
                     (int64_t) cameraPos.z);
      if (CameraLight.setPos(cameraVoxel)) {
        Renderer.recreateSurrounding(cameraVoxel);
      }
    }


    if (controls.leftMousePoll()) {
      for (int i = 0; i < 20; ++i) {
        auto cameraPos = camera.getPosition() + camera.getDirection(i / 4.0f);

        v3 cameraVoxel((int64_t) cameraPos.x, (int64_t) cameraPos.y,
                       (int64_t) cameraPos.z);

        if (Chunk.get(cameraVoxel).isFree())
          continue;

        Chunk.setBlock(cameraVoxel, Voxel::AIR);

        Renderer.recreateSurrounding(cameraVoxel);
        break;
      }
    }

    if (controls.rightMousePoll()) {
      vec3 lastFreePos = camera.getPosition();
      for (int i = 0; i < 20; ++i) {
        auto testPos = camera.getPosition() + camera.getDirection(i / 4.0f);

        v3 cameraVoxel((int64_t) testPos.x, (int64_t) testPos.y,
                       (int64_t) testPos.z);

        if (Chunk.get(cameraVoxel).isFree()) {
          lastFreePos = testPos;
          continue;
        }

        if (lastFreePos != camera.getPosition()) {
          v3 lastFreeVoxel((int64_t) lastFreePos.x, (int64_t) lastFreePos.y,
                         (int64_t) lastFreePos.z);
          Chunk.setBlock(lastFreeVoxel, SelectedType);

          Renderer.recreateSurrounding(cameraVoxel);
        }
        break;

      }
    }

  } // Check if the ESC key was pressed or the window was closed
  while (run);

  // Cleanup VBO and shader
  glDeleteProgram(BlockProgramID);
  glDeleteTextures(1, &TextureID);

  glDeleteProgram(SpaceProgramID);
  glDeleteTextures(1, &SpaceTextureID);

  return 0;
}

