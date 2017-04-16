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
#include "Voxel.h"
#include "VoxelChunk.h"
#include "stb_perlin.h"

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


class Space {

  std::vector<VoxelChunk*> Chunks;

  Voxel defaultVoxel;

public:
  Space() {
  }

  void add(VoxelChunk &NewChunk) {
    Chunks.push_back(&NewChunk);
  }

  VoxelChunk* getChunk(const v3 &pos) {
    for (auto Chunk : Chunks) {
      if (Chunk->contains(pos)) {
        return Chunk;
      }
    }
    return nullptr;
  }

  Voxel& get(const v3& pos) {
    if (auto Chunk = getChunk(pos)) {
      return Chunk->get(pos);
    }

    defaultVoxel = Voxel();
    return defaultVoxel;
  }

  bool isGravityAffected(v3 pos) {
    for (int i = 0; i < 10; ++i) {
      Voxel &V = get(pos);
      if (V.is(Voxel::STEEL_FLOOR))
        return true;
      pos.y--;
    }
    return false;
  }
};


#endif //TUTORIALS_MAP_H
