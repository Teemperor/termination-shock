#ifndef TUTORIALS_MAP_H
#define TUTORIALS_MAP_H


#include <cstdint>
#include <cassert>
#include <limits>
#include <utility>
#include <algorithm>
#include <vector>
#include <random>
#include <iostream>
#include <stack>
#include <tuple>
#include <set>
#include <GL/glew.h>
#include <unordered_set>
#include <math.h>


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


  double length() const {
    return std::sqrt(x * x + y * y + z * z);
  }

  double distance(const v3&other) const {
    v3 diff = other - *this;
    return diff.length();
  }

  friend std::ostream &operator<<(std::ostream &os, v3 const &v) {
    return os << v.x << ", " << v.y << ", " << v.z;
  }
};


namespace std {
  template <> struct hash<v3>
  {
    size_t operator()(const v3 & v) const
    {
      return (size_t) (v.x ^ v.y ^ v.z);
    }
  };
}

struct v3f {
  float x, y, z;

  v3f() {

  }
  v3f(float x, float y, float z) : x(x), y(y), z(z) {
  }

  v3 toVoxelPos() {
    return {(int64_t) x, (int64_t) y, (int64_t) z};
  }
};


class Voxel {
  uint16_t Light = 10;
  uint8_t Type = 0;
  unsigned Marked : 1;

public:

  static constexpr uint8_t LightMin = 10;
  static constexpr float TEX_SIZE = 1.0f / 16;

  enum Types {
    SPACE,
    
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
    /*noise::module::Perlin myModule;

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
     */
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
    assert(false);
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

  void spreadLight(v3 startPos, const bool increase, uint8_t light = 255) {
    std::vector<v3> StorageA = {startPos};
    StorageA.reserve(500);
    std::vector<v3> StorageB;
    StorageB.reserve(500);
    std::vector<v3> *ToHandle = &StorageA;
    std::vector<v3> *ToHandleNext = &StorageB;

    const int64_t maxLightDistance = 8;

    for (int64_t x = -maxLightDistance + startPos.x; x <= maxLightDistance + startPos.x; ++x) {
      for (int64_t y = -maxLightDistance + startPos.y; y <= maxLightDistance + startPos.y; ++y) {
        for (int64_t z = -maxLightDistance + startPos.z; z <= maxLightDistance + startPos.z; ++z) {
          get({x, y, z}).mark(false);
        }
      }
    }

    const static float fallingFactor = 0.40f;

    int Distance = 0;
    while (true) {

      if (ToHandle->empty()) {
        ++Distance;
        if (ToHandleNext->empty())
          break;
        if ((light * fallingFactor) <= Voxel::LightMin)
          break;
        if (Distance > 4)
          light *= fallingFactor;
        std::swap(ToHandle, ToHandleNext);
      }
      v3 pos = ToHandle->back();
      ToHandle->pop_back();

      Voxel& C = get(pos);

      if (increase)
        C.increaseLight(light);
      else
        C.decreaseLight(light);

      for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
          for (int z = -1; z <= 1; ++z) {
            v3 iterPos(pos.x + x, pos.y + y, pos.z + z);

            if (x == 0 && z == 0 && y == 0)
              continue;

            Voxel& V = get(iterPos);
            if (V.blocksView())
              continue;

            if (!V.marked()) {
              V.mark(true);
              ToHandleNext->push_back(iterPos);
            }
          }
        }
      }
    }
  }

  void generateSpaceShip() {
    for (int64_t x = 50; x <= 60; ++x) {
      for (int64_t z = 50; z <= 60; ++z) {
        get({x, 11, z}) = Voxel::STEEL_FLOOR;
        get({x, 19, z}) = Voxel::METAL_CEILING;

        if (x % 10 == 0 || z % 10 == 0) {
          get({x, 18, z}) = Voxel::METAL_WALL;
          get({x, 17, z}) = Voxel::METAL_WALL;
          get({x, 16, z}) = Voxel::METAL_WALL;
          get({x, 15, z}) = Voxel::METAL_WALL;
          get({x, 14, z}) = Voxel::METAL_WALL;
          get({x, 13, z}) = Voxel::GLASS;
          get({x, 12, z}) = Voxel::METAL_WALL;
        }
      }
    }

    get({55, 15, 55}) = Voxel::LAMP;
    lights.insert({55, 15, 55});

    relight();
  }

  Voxel Default;

  std::unordered_set<v3> lights;

  float spaceRecalcTimer = 10;
  float timeSinceLastSpaceRecalc = 9;

  void spreadSpace(v3 startPos) {
    std::vector<v3> ToHandle = {startPos};
    ToHandle.reserve(size.x * size.y * size.z);

    while (!ToHandle.empty()) {

      if (ToHandle.size() > (128 * 128 * 128)) {
        std::cout << "TOO big" << std::endl;
        return;
      }

      v3 pos = ToHandle.back();
      ToHandle.pop_back();

      Voxel& C = get(pos);

      C.transform(Voxel::SPACE);

      static const std::array<v3, 6> Offsets = {
        v3(0, 0, 1),
        v3(0, 0, -1),
        v3(0, 1, 0),
        v3(0, -1, 0),
        v3(1, 0, 0),
        v3(-1, 0, 0),
      };

      for (int i = 0; i < Offsets.size(); ++i) {
        v3 o = Offsets[i];
        v3 iterPos(pos.x + o.x, pos.y + o.y, pos.z + o.z);

        if (iterPos.x < 0 || iterPos.x >= size.x)
          continue;
        if (iterPos.y < 0 || iterPos.y >= size.y)
          continue;
        if (iterPos.z < 0 || iterPos.z >= size.z)
          continue;

        Voxel& V = get(iterPos);
        if (!V.is(Voxel::AIR))
          continue;

        if (V.marked())
          continue;

        V.mark(true);

        ToHandle.push_back(iterPos);
      }
    }
  }

  void recalcSpace() {
    //return; // TODO
    for (int64_t x = offset.x; x < size.x + offset.x; ++x) {
      for (int64_t y = offset.y; y < size.y + offset.y; ++y) {
        for (int64_t z = offset.z; z < size.z + offset.z; ++z) {
          Voxel &V = get({x, y, z});
          if (V.is(Voxel::SPACE)) {
            V.transform(Voxel::AIR);
          }
          V.mark(false);
        }
      }
    }
    spreadSpace({0, 0, 0});
    assert(get({0, 0, 0}).is(Voxel::SPACE));
  }


public:
  VoxelChunk(v3 offset) : offset(offset), engine(11), distPercent(0, 1) {
    size = {128, 128, 128};
    Voxels.resize(size.x * size.y * size.z, Voxel::SPACE);
    generateSpaceShip();
  }

  void update(float deltaTime) {
    timeSinceLastSpaceRecalc += deltaTime;
    if (timeSinceLastSpaceRecalc >= spaceRecalcTimer) {
      timeSinceLastSpaceRecalc -= spaceRecalcTimer;
      recalcSpace();
    }
  }

  void addLight(v3 pos) {
    lights.insert(pos);
  }

  void moveLight(v3 oldPos, v3 newPos) {
    lights.erase(oldPos);
    lights.insert(newPos);
    spreadLight(oldPos, false);
    spreadLight(newPos, true);
  }

  void setBlock(v3 pos, Voxel newV) {
    for (auto &light : lights) {
      if (light.distance(pos) < 8) {
        spreadLight(light, false);
      }
    }

    Voxel& V = get(pos);
    V = newV;


    for (auto &light : lights) {
      if (light.distance(pos) < 8) {
        spreadLight(light, true);
      }
    }
  }

  void relight() {
    for (auto &light : lights) {
      spreadLight(light, true);
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

  bool isGravityAffected(v3 pos) {
    for (int i = 0; i < 10; ++i) {
      if (get(pos).is(Voxel::STEEL_FLOOR))
        return true;
      pos.y--;
    }
    return false;
  }
};

class MovingLight {
  VoxelChunk *Chunk;
  v3 pos;
public:
  MovingLight(VoxelChunk *Chunk, v3 pos = {0, 0, 0}) : Chunk(Chunk), pos(pos) {
    Chunk->addLight(pos);
  }

  bool setPos(v3 newPos) {
    if (newPos != pos) {
      Chunk->moveLight(pos, newPos);
      pos = newPos;
      return true;
    }
    return false;
  }
};

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

class MovingEntity : public Entity {
  VoxelChunk *Chunk;

  v3f vel;

  bool isHeightGood(float h) {
    static const float r = 0.3f;
  return
      Chunk->get(v3f(pos.x + r, pos.y + h, pos.z - r).toVoxelPos()).isFree() &&
      Chunk->get(v3f(pos.x - r, pos.y + h, pos.z - r).toVoxelPos()).isFree() &&
      Chunk->get(v3f(pos.x + r, pos.y + h, pos.z + r).toVoxelPos()).isFree() &&
      Chunk->get(v3f(pos.x - r, pos.y + h, pos.z + r).toVoxelPos()).isFree();
  }

  bool gravityAffected() const {
    return Chunk->isGravityAffected(v3f(pos.x, pos.y - 2.0f, pos.z).toVoxelPos());
  }

  bool isPosGood() {
    return isHeightGood(-2.0f) &&isHeightGood(-1.5f) && isHeightGood(-0.7f) && isHeightGood(0) && isHeightGood(0.3f);
  }

  bool tryIncrease(float& target, float value) {
    float backup = target;
    target += value;
    if (!isPosGood()) {
      target = backup;
      return false;
    }
    return true;
  }

public:
  MovingEntity(VoxelChunk *Chunk) : Chunk(Chunk), vel(0, 0, 0) {
    vel.y = -0.4f;
  }

  void setMove(float hRot, float dx, float dy, float dz) {
    const float speed = 4.5;
    rot = hRot;
    float moveRot = rot;
    if (std::abs(dx) > 0.1f || std::abs(dz) > 0.1f) {
    //  moveRot += std::atan2(dz, dx) - PI / 2;
    }

    float runSpeedMod = 1;
    if (!gravityAffected())
      runSpeedMod = 0.5f;

    vel.x = std::sin(moveRot) * dz * speed * runSpeedMod;
    vel.x -= std::cos(moveRot) * dx * speed * runSpeedMod;
    vel.z = std::cos(moveRot) * dz * speed * runSpeedMod;
    vel.z += std::sin(moveRot) * dx * speed * runSpeedMod;
    if (!gravityAffected())
      vel.y = dy * speed * 0.5f;
  }

  void jump() {
    if (gravityAffected())
      vel.y = 7.0f;
  }

  void update(float dtime) {
    tryIncrease(pos.x, vel.x * dtime);
    if (gravityAffected()) {
      if (tryIncrease(pos.y, vel.y * dtime)) {
        vel.y -= 20.0f * dtime;
      } else {
        vel.y = -0.1f;
      }
    } else {
      tryIncrease(pos.y, vel.y * dtime);
    }
    tryIncrease(pos.z, vel.z * dtime);
  }

  bool onGround() {
    return !isHeightGood(-2.2f);
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
