#ifndef TUTORIALS_MAP_H
#define TUTORIALS_MAP_H


#include <cstdint>
#include <cassert>
#include <limits>
#include <utility>
#include <algorithm>
#include <vector>
#include <noise/module/perlin.h>
#include <random>
#include <iostream>
#include <stack>
#include <tuple>
#include <set>
#include <GL/glew.h>

struct v3 {
  int64_t x, y, z;

  v3() {
  }
  v3(int64_t x, int64_t y, int64_t z) : x(x), y(y), z(z) {}

  bool operator!=(const v3 &Other) const {
    return std::tie(x, y, z) != std::tie(Other.x, Other.y, Other.z);
  }
  bool operator==(const v3 &Other) const {
    return std::tie(x, y, z) == std::tie(Other.x, Other.y, Other.z);
  }

  bool operator<(const v3 &Other) const {
    return std::tie(x, y, z) < std::tie(Other.x, Other.y, Other.z);
  }

  v3 operator*(const int64_t Scale) const {
    v3 result = *this;
    result *= Scale;
    return result;
  }

  v3 operator+(const v3&Other) const {
    v3 result = *this;
    result += Other;
    return result;
  }

  v3 operator-(const v3&Other) const {
    v3 result = *this;
    result -= Other;
    return result;
  }

  v3& operator-=(const v3&Other) {
    x -= Other.x;
    y -= Other.y;
    z -= Other.z;
    return *this;
  }

  v3& operator+=(const v3&Other) {
    x += Other.x;
    y += Other.y;
    z += Other.z;
    return *this;
  }

  v3& operator*=(const int64_t Scale) {
    x *= Scale;
    y *= Scale;
    z *= Scale;
    return *this;
  }
};

struct v3f {
  float x, y, z;

  v3f(float x, float y, float z) : x(x), y(y), z(z) {
  }
};


class Voxel {
  uint8_t Type = 0;
  uint8_t Light = 0;
public:

  static constexpr uint8_t LightMin = 0;
  static constexpr float TEX_SIZE = 1.0f / 16;

  enum Types {
    AIR,
    GRASS,
    STONE,
    EARTH,
    BEDROCK,
    TREE,
    LEAF,

    SPACE,
    STEEL_WALL,
    STEEL_FLOOR,
    METAL_WALL,
    METAL_CEILING,
    LAMP,
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

  void increaseLight(uint8_t addLight) {
    Light = (uint8_t) std::min(255u, (unsigned) Light + (unsigned)addLight);
  }

  bool isDark() const {
    return Light <= LightMin;
  }

  float lightPercent() const {
    return ((float) Light) / std::numeric_limits<uint8_t>::max();
  }

  uint8_t light() const {
    return Light;
  }

  bool is(Types T) const {
    return Type == T;
  }

  bool isFree() const {
    return Type == AIR || Type == SPACE;
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
        return std::make_pair(1 * TEX_SIZE, 0);
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


      case STEEL_WALL:
        return std::make_pair(0 * TEX_SIZE, 0);
      case  STEEL_FLOOR:
        return std::make_pair(1 * TEX_SIZE, 0);
      case  METAL_WALL:
        return std::make_pair(2 * TEX_SIZE, 0);
      case  METAL_CEILING:
        return std::make_pair(3 * TEX_SIZE, 0);
      case LAMP:
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


class VoxelChunk {

  v3 offset;
  v3 size = v3(getSize(), getSize(), getSize());
  std::vector<Voxel> Voxels;

  std::default_random_engine engine;
  std::uniform_real_distribution<float> distPercent;

  float randomPercent() {
    return distPercent(engine);
  }

  void plantTree(v3 pos, int h) {
    for (int hi = 0; hi < h; ++hi) {
      get({pos.x, pos.y + hi, pos.z}) = Voxel::TREE;
    }
    get({pos.x, pos.y + h, pos.z}) = Voxel::LEAF;
    for (int x = -1; x <= 1; ++x) {
      for (int z = -1; z <= 1; ++z) {
        if (x == 0 && z == 0)
          continue;
        get({pos.x + x, pos.y + h - 1, pos.z + z}) = Voxel::LEAF;
      }
    }
  }

  void plantTrees() {
    for (int64_t x = offset.x; x < size.x + offset.x; ++x) {
      for (int64_t y = offset.y; y < size.y + offset.y; ++y) {
        for (int64_t z = offset.z; z < size.z + offset.z; ++z) {
          auto V = getAnnotated({x, y, z});
          if (V.V.is(Voxel::GRASS) && V.S[0].isFree()) {
            if (randomPercent() > 0.97f)
              plantTree({x, y + 1, z}, (int) (2 + randomPercent() * 5));
          }
        }
      }
    }
  }

  void generate() {
    noise::module::Perlin myModule;

    for (int64_t x = offset.x; x < size.x + offset.x; ++x) {
      const double factor = 100.0;
        for (int64_t z = offset.z; z < size.z + offset.z; ++z) {
        double d = myModule.GetValue(x / factor, z / factor, 0);
        double d2 = myModule.GetValue(x / factor, z / factor, 1000);
        int64_t h = (int64_t) (size.y / 2 + d * size.y / 5);
        for (int64_t y = 1; y < h; ++y) {
          get({x, y, z}) = Voxel::GRASS;
        }

        int64_t h2 = (int64_t) (size.y / 4 + d * size.y / 5);
        for (int64_t y = 1; y < h2; ++y) {
          get({x, y, z}) = Voxel::STONE;
        }

        get({x, 0, z}) = Voxel::BEDROCK;
      }
    }

    for (int64_t x = offset.x; x < size.x + offset.x; ++x) {
      const double factor = 20.0;
      for (int64_t y = offset.y; y < size.y + offset.y; ++y) {
        for (int64_t z = offset.z; z < size.z + offset.z; ++z) {
          double d = myModule.GetValue(x / factor, y / factor, z / factor);
          if (d > 0.6) {
            get({x, y, z}) = Voxel(Voxel::AIR);
          }
        }
      }
    }
    plantTrees();

    createLight();
    for (int64_t x = offset.x; x < size.x + offset.x; ++x) {
      for (int64_t y = offset.y; y < size.y + offset.y; ++y) {
        for (int64_t z = offset.z; z < size.z + offset.z; ++z) {
          auto A = getAnnotated({x, y, z});
          if (A.V.is(Voxel::GRASS) && A.S[0].lightPercent() < 0.5f) {
            get({x, y, z}) = Voxel(Voxel::EARTH);
          }
        }
      }
    }
  }

  void createLight() {
    for (int64_t x = offset.x; x < size.x + offset.x; ++x) {
      for (int64_t z = offset.z; z < size.z + offset.z; ++z) {
        for (int64_t y = offset.y + size.y - 1;; --y) {
          auto& V = get({x, y, z});
          if (V.isFree())
            V.setLight(255);
          else
            break;
          if (y == offset.y)
            break;
        }
      }
    }
    while (spreadLight());
  }

  bool spreadLight() {
    bool hasChanged = false;
    for (int64_t x = offset.x; x < size.x + offset.x; ++x) {
      for (int64_t y = offset.y; y < size.y + offset.y; ++y) {
        for (int64_t z = offset.z; z < size.z + offset.z; ++z) {
          auto V = getAnnotated({x, y, z});
          if (V.V.isFree()) {
            unsigned newLight = (unsigned) (V.surroundLight() * 0.90f) + V.V.light();
            newLight = std::min(255u, newLight);
            if (newLight > 0) {
              get({x, y, z}).setLight((uint8_t) newLight);
              hasChanged = true;
            }
          }
        }
      }
    }
    return hasChanged;
  }

  void spreadLight(v3 startPos, uint8_t light = 255) {
    std::vector<v3> ToHandle = {startPos};
    std::vector<v3> ToHandleNext;
    std::set<v3> Handled;

    while (true) {
      if (ToHandle.empty()) {
        if (ToHandleNext.empty())
          break;
        if (light < 10)
          break;
        light *= 0.60;
        if (light <= Voxel::LightMin) {
          break;
        }
        std::swap(ToHandle, ToHandleNext);
      }
      v3 pos = ToHandle.back();
      ToHandle.pop_back();

      get(pos).increaseLight(light);

      static const std::array<v3, 6> Offsets = {
        v3(0, 0, 1),
        v3(0, 0, -1),
        v3(0, 1, 0),
        v3(0, -1, 0),
        v3(1, 0, 0),
        v3(-1, 0, 0),
      };

      for (int i = 0; i < 6; ++i) {
        v3 o = Offsets[i];
        v3 iterPos(pos.x + o.x, pos.y + o.y, pos.z + o.z);

        if (!get(iterPos).isFree())
          continue;
        if (light < 10)
          continue;
        if (Handled.find(iterPos) == Handled.end()) {
          ToHandleNext.push_back(iterPos);
          Handled.insert(iterPos);
        }
      }
    }
  }

  void generateSpaceShip() {
    for (int64_t x = 15; x < 90; ++x) {
      for (int64_t z = 15; z < 90; ++z) {
        get({x, 11, z}) = Voxel::STEEL_FLOOR;
        get({x, 15, z}) = Voxel::METAL_CEILING;

        if (x % 9 == 0 || z % 9 == 0) {
          if (x % 9 != 4 && x % 9 != 5) {
            get({x, 14, z}) = Voxel::METAL_WALL;
            get({x, 13, z}) = Voxel::METAL_WALL;
            get({x, 12, z}) = Voxel::METAL_WALL;
          }
        }
      }
    }
    relight();
  }

  Voxel Default;

public:
  VoxelChunk(v3 offset) : offset(offset), engine(11), distPercent(0, 1) {
    size = {128, 128, 128};
    Voxels.resize(size.x * size.y * size.z);
    generateSpaceShip();
  }

  void relight() {
    for (int64_t x = offset.x; x < size.x + offset.x; ++x) {
      for (int64_t y = offset.y; y < size.y + offset.y; ++y) {
        for (int64_t z = offset.z; z < size.z + offset.z; ++z) {
          get({x, y, z}).setLight(0);
        }
      }
    }
    for (int64_t x = 45; x < 60; ++x) {
      for (int64_t z = 45; z < 60; ++z) {

        if (x % 9 == 4 && z % 9 == 4) {
          spreadLight({x, 14, z});
          spreadLight({x, 16, z});
        }
      }
    }
  }

  Voxel& get(v3 pos) {
    pos -= offset;
    if (pos.x < 0 || pos.x >= size.x) {
      Default = Voxel();
      return Default;
    }
    if (pos.y < 0 || pos.y >= size.y) {
      Default = Voxel();
      return Default;
    }
    if (pos.z < 0 || pos.z >= size.z) {
      Default = Voxel();
      return Default;
    }
    return Voxels[pos.x + pos.y * size.x + pos.z * size.x * size.y];
  }

  AnnotatedVoxel getAnnotated(v3 pos) {
    AnnotatedVoxel Result;
    Result.V = get(pos);
    Result.S[0] = get({pos.x, pos.y + 1, pos.z});
    Result.S[1] = get({pos.x, pos.y - 1, pos.z});
    Result.S[2] = get({pos.x, pos.y, pos.z - 1});
    Result.S[3] = get({pos.x, pos.y, pos.z + 1});
    Result.S[4] = get({pos.x - 1, pos.y, pos.z});
    Result.S[5] = get({pos.x + 1, pos.y, pos.z});
    return Result;
  }

  static size_t getSize() {
    return 64;
  }

  const v3& getOffset() const {
    return offset;
  }

};

class VoxelMap {
public:

};

class VoxelRaycast {
public:
  VoxelRaycast(const v3f& start, const v3f& direction) {

  }
};

#endif //TUTORIALS_MAP_H
