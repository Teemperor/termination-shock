#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include "GL/gl.h"

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
#include <cassert>

inline void split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (getline(ss, item, delim)) {
    elems.push_back(item);
  }
}

inline std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
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

extern TextureManager TexMgr;

#endif // TEXTURE_H
