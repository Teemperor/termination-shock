#include "Texture.h"

const std::string &TextureID::getPath() {
  return TexMgr.getName(ID);
}

TextureManager TexMgr;

void TextureID::activate() {
  TexMgr[ID].activate();
}
