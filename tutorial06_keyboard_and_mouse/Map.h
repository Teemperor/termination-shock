#ifndef TUTORIALS_MAP_H
#define TUTORIALS_MAP_H


#include <cstdint>
#include <cassert>
#include <limits>
#include <utility>
#include <algorithm>
#include <vector>
#include <noise/module/perlin.h>

struct v3 {
  int64_t x, y, z;
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
    BEDROCK
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
    return Type == AIR;
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
      case BEDROCK:
        return std::make_pair(6 * TEX_SIZE, 3 * TEX_SIZE);
      case EARTH:
        return std::make_pair(2 * TEX_SIZE, 0);
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

  v3 size;
  std::vector<Voxel> Voxels;

  void generate() {
    noise::module::Perlin myModule;

    for (int64_t x = 0; x < size.x; ++x) {
      const double factor = 100.0;
      for (int64_t z = 0; z < size.z; ++z) {
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

    for (int64_t x = 0; x < size.x; ++x) {
      const double factor = 20.0;
      for (int64_t y = 1; y < size.y; ++y) {
        for (int64_t z = 0; z < size.z; ++z) {
          double d = myModule.GetValue(x / factor, y / factor, z / factor);
          if (d > 0.6) {
            get({x, y, z}) = Voxel(Voxel::AIR);
          }
        }
      }
    }
    createLight();
    for (int64_t x = 0; x < size.x; ++x) {
      for (int64_t y = 1; y < size.y; ++y) {
        for (int64_t z = 0; z < size.z; ++z) {
          auto A = getAnnotated({x, y, z});
          if (A.V.is(Voxel::GRASS) && A.S[0].lightPercent() < 0.5f) {
            get({x, y, z}) = Voxel(Voxel::EARTH);
          }
        }
      }
    }
  }

  void createLight() {
    for (int64_t x = 0; x < size.x; ++x) {
      for (int64_t z = 0; z < size.z; ++z) {
        for (int64_t y = size.y - 1;; --y) {
          auto& V = get({x, y, z});
          if (V.isFree())
            V.setLight(255);
          else
            break;
          if (y == 0)
            break;
        }
      }
    }
    while (spreadLight());
  }

  bool spreadLight() {
    bool hasChanged = false;
    for (int64_t x = 0; x < size.x; ++x) {
      for (int64_t y = 0; y < size.y; ++y) {
        for (int64_t z = 0; z < size.z; ++z) {
          auto V = getAnnotated({x, y, z});
          if (V.V.isFree() && V.V.isDark()) {
            uint8_t newLight = (uint8_t) (V.surroundLight() * 0.90f);
            if (newLight > 0) {
              get({x, y, z}).setLight(newLight);
              hasChanged = true;
            }
          }
        }
      }
    }
    return hasChanged;
  }

  Voxel Default;

public:
  VoxelChunk(v3 size) : size(size) {
    Voxels.resize(size.x * size.y * size.z);
    generate();
  }

  Voxel& get(v3 pos) {
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

  const v3& getSize() {
    return size;
  }

};

class VoxelMap {
public:

};

#endif //TUTORIALS_MAP_H
