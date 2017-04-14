#ifndef BLOCKSIDEARRAY_H
#define BLOCKSIDEARRAY_H


#include <vector>
#include <string>
#include "Texture.h"

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
    TexMgr.loadTexture(TexturePath)) {
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

#endif // BLOCKSIDEARRAY_H
