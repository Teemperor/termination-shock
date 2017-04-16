#ifndef ENTITY_H
#define ENTITY_H

#include "v3.h"

class Space;

class Entity {
protected:
  v3f pos;
  float rot = 0;

public:
  Entity() : pos(55, 14, 55) {

  }

  v3f position() const {
    return pos;
  }

  void setRotation(float value) {
    rot = value;
  }

  float rotation() const {
    return rot;
  }
};

#endif // ENTITY_H
