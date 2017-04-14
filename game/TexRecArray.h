#ifndef TEXRECARRAY_H
#define TEXRECARRAY_H

#include "Texture.h"
#include "Rec.h"

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
    TexMgr.loadTexture(TexturePath)) {
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


#endif // TEXRECARRAY_H
