#ifndef TUTORIALS_VOXEL_H
#define TUTORIALS_VOXEL_H

#include <cstdint>
#include <cassert>
#include <limits>
#include <algorithm>
#include "v3.h"

class VoxelChunk;

class Voxel {
  uint16_t Light = 10;
  uint8_t Type = 0;
  unsigned Marked : 1;

public:

  static constexpr uint8_t LightMin = 10;
  static constexpr float TEX_SIZE = 1.0f / 16;

  enum Types {
    SPACE = 0,

    AIR,
    GRASS,
    STONE,
    EARTH,
    BEDROCK,
    TREE,
    LEAF,

    STEEL_WALL,
    STEEL_FLOOR,
    METAL_WALL,
    METAL_CEILING,
    LAMP,
    GENERATOR,
    CRATE,
    GLASS,
    AIRLOCK

  };
  Voxel() {
    assert(isDark());
    assert(isFree());
  }
  Voxel(Types t) : Type(t) {
    assert(isDark());
  }

  void setLight(uint8_t L) {
    if (L < LightMin)
      return;
    Light = L;
  }

  void mark(bool L) {
    Marked = L ? 1 : 0;
  }

  bool marked() const {
    return Marked != 0;
  }

  void increaseLight(uint16_t addLight) {
    Light += addLight;
  }

  void decreaseLight(uint16_t addLight) {
    Light -= addLight;
  }

  bool isDark() const {
    return Light <= LightMin;
  }

  void transform(Types T) {
    Type = T;
  }

  bool transparent() const {
    return Type == SPACE || Type == AIR;
  }

  bool blocksView() const {
    switch(Type) {
      case GLASS:
      case SPACE:
      case AIR:
      case AIRLOCK:
        return false;
      default:
        return true;
    }
  }

  float lightPercent() const {
    return ((float) light()) / std::numeric_limits<uint8_t>::max();
  }

  uint8_t light() const {
    return (uint8_t) std::min<uint16_t>(std::numeric_limits<uint8_t>::max(), Light);
  }

  bool is(Types T) const {
    return Type == T;
  }

  bool isBuildable() const {
    return Type != AIR && Type != SPACE;
  }

  bool isFree() const {
    return Type == AIR || Type == SPACE|| Type == AIRLOCK;
  }

  void callback(bool added, const v3 &pos, VoxelChunk &Chunk);


#define VOXEL_NAME_MACRO(ENUM) case ENUM : return #ENUM

  const char *getName() const {
    switch(Type) {
      VOXEL_NAME_MACRO(SPACE);
      VOXEL_NAME_MACRO(AIR);
      default: return "Unknown voxel";
    }
  }

  std::pair<float, float> getUVOffset(unsigned side, bool airAbove) const {
    assert(Type != AIR);
    switch (Type) {
      case GRASS:
        switch (side) {
          case 0:
            return std::make_pair(0, 0);
          case 1:
            return std::make_pair(2 * TEX_SIZE, 0);
          default:
            if (airAbove)
              return std::make_pair(3 * TEX_SIZE, 0);
            return std::make_pair(2 * TEX_SIZE, 0);
        }
      case STONE:
        return std::make_pair(6 * TEX_SIZE, 0);
      case TREE:
        switch (side) {
          case 0:
          case 1:
            return std::make_pair(5 * TEX_SIZE, 1 * TEX_SIZE);
          default:
            return std::make_pair(4 * TEX_SIZE, 1 * TEX_SIZE);
        }
      case LEAF:
        return std::make_pair(5 * TEX_SIZE, 3 * TEX_SIZE);
      case BEDROCK:
        return std::make_pair(1 * TEX_SIZE, 1 * TEX_SIZE);
      case EARTH:
        return std::make_pair(2 * TEX_SIZE, 0);


      case CRATE:
        switch (side) {
          case 0:
          case 1:
            return std::make_pair(1 * TEX_SIZE, 1 * TEX_SIZE);
          default:
            return std::make_pair(2 * TEX_SIZE, 1 * TEX_SIZE);
        }
      case STEEL_WALL:
        return std::make_pair(0 * TEX_SIZE, 0);
      case GENERATOR:
        return std::make_pair(0, 1  * TEX_SIZE);
      case  STEEL_FLOOR:
        return std::make_pair(1 * TEX_SIZE, 0);
      case  METAL_WALL:
        return std::make_pair(2 * TEX_SIZE, 0);
      case  METAL_CEILING:
        return std::make_pair(3 * TEX_SIZE, 0);
      case  GLASS:
        return std::make_pair(3 * TEX_SIZE, 1 * TEX_SIZE);
      case LAMP:
        return std::make_pair(4 * TEX_SIZE, 0);
      case AIRLOCK:
        return std::make_pair(4 * TEX_SIZE, 0);

      default: assert(false);
        return std::make_pair(0, 0);
    }
  };

};


struct AnnotatedVoxel {
  Voxel V;
  // first top, second below
  Voxel S[6];

  std::pair<float, float> getUVOffset(unsigned side) const {
    return V.getUVOffset(side, S[0].isFree());
  };

  uint8_t surroundLight() const {
    uint8_t Result = 0;
    for (int i = 0; i < 6; ++i)
      Result = std::max(Result, S[i].light());
    return Result;
  }
};

#endif //TUTORIALS_VOXEL_H
