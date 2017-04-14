#ifndef VOXELMAPRENDERER_H
#define VOXELMAPRENDERER_H

#include "Voxel.h"
#include "Map.h"
#include "v3.h"
#include "BlockSideArray.h"

#define ADD_VOXEL_SIDE(Ax, Ay, Az, Bx, By, Bz, Cx, Cy, Cz, Dx, Dy, Dz, side) \
  if (!V.S[side].blocksView()){                                    \
    GLfloat u = V.getUVOffset(side).first;                         \
    GLfloat v = V.getUVOffset(side).second;                        \
    Array.add({   (float) Ax, (float) Ay, (float) Az,              \
                  (float) Bx, (float) By, (float) Bz,              \
                  (float) Cx, (float) Cy, (float) Cz,              \
                  (float) Cx, (float) Cy, (float) Cz,              \
                  (float) Dx, (float) Dy, (float) Dz,              \
                  (float) Ax, (float) Ay, (float) Az}, {           \
                  u, v,                                            \
                  u + us,   v,                                     \
                  u + us,   v + vs,                                \
                  u + us,   v + vs,                                \
                  u, v + vs,                                       \
                  u, v                                             \
                  }, V.S[side].lightPercent(),                     \
                  getOcclusionLighting({x, y, z}, side));          \
  }

class VoxelMapRenderer {

  VoxelChunk *Map;
  BlockSideArray Array;

  v3 offset;
  v3 size = v3(getSize(), getSize(), getSize());

  static constexpr float ONE_THIRD = 1.0f / 3.0f;

#define LIGHT_SUM(ax, ay, az, bx, by, bz, cx, cy, cz) \
   (Map->get({pos.x + ax, pos.y + ay, pos.z + az}).transparent() ? ONE_THIRD : 0.1f) \
 + (Map->get({pos.x + bx, pos.y + by, pos.z + bz}).transparent() ? ONE_THIRD : 0.1f) \
 + (Map->get({pos.x + cx, pos.y + cy, pos.z + cz}).transparent() ? ONE_THIRD : 0.1f);

  std::array<GLfloat, 6> getOcclusionLighting(const v3& pos, unsigned side) {
    std::array<GLfloat, 6> Result = {1, 1, 1, 1, 1, 1};
    switch(side) {
      case 0: {
        Result[2] = Result[3] =  LIGHT_SUM( 1,  1,  0,
                                            1,  1,  1,
                                            0,  1,  1);
        Result[0] = Result[5] =  LIGHT_SUM(-1,  1,  0,
                                           -1,  1, -1,
                                            0,  1, -1);
        Result[1] =              LIGHT_SUM(-1,  1,  0,
                                           -1,  1,  1,
                                            0,  1,  1);
        Result[4] =              LIGHT_SUM( 1,  1,  0,
                                            1,  1, -1,
                                            0,  1, -1);
        break;
      }
      case 1: {
        Result[2] = Result[3] =  LIGHT_SUM( 1, -1,  0,
                                            1, -1,  1,
                                            0, -1,  1);
        Result[0] = Result[5] =  LIGHT_SUM(-1, -1,  0,
                                           -1, -1, -1,
                                            0, -1, -1);
        Result[1] =              LIGHT_SUM(-1, -1,  0,
                                           -1, -1,  1,
                                            0, -1,  1);
        Result[4] =              LIGHT_SUM( 1, -1,  0,
                                            1, -1, -1,
                                            0, -1, -1);
        break;
      }
      case 2: {
        Result[2] = Result[3] =  LIGHT_SUM( 1,  0, -1,
                                            0, -1, -1,
                                            1, -1, -1);
        Result[0] = Result[5] =  LIGHT_SUM(-1,  0, -1,
                                            0,  1, -1,
                                           -1,  1, -1);
        Result[1] =              LIGHT_SUM( 1,  0, -1,
                                            0,  1, -1,
                                            1,  1, -1);
        Result[4] =              LIGHT_SUM(-1,  0, -1,
                                            0, -1, -1,
                                           -1, -1, -1);
        break;
      }
      case 3: {
        Result[2] = Result[3] =  LIGHT_SUM( 1,  0,  1,
                                            0, -1,  1,
                                            1, -1,  1);
        Result[0] = Result[5] =  LIGHT_SUM(-1,  0,  1,
                                            0,  1,  1,
                                           -1,  1,  1);
        Result[1] =              LIGHT_SUM( 1,  0,  1,
                                            0,  1,  1,
                                            1,  1,  1);
        Result[4] =              LIGHT_SUM(-1,  0,  1,
                                            0, -1,  1,
                                           -1, -1,  1);
        break;
      }
      case 4: {
        Result[2] = Result[3] =  LIGHT_SUM(-1, -1, -1,
                                           -1,  0, -1,
                                           -1, -1,  0);
        Result[0] = Result[5] =  LIGHT_SUM(-1,  1,  1,
                                           -1,  0,  1,
                                           -1,  1,  0);
        Result[1] =              LIGHT_SUM(-1,  1, -1,
                                           -1,  0, -1,
                                           -1,  1,  0);
        Result[4] =              LIGHT_SUM(-1, -1,  1,
                                           -1,  0,  1,
                                           -1, -1,  0);
        break;
      }
      case 5: {
        Result[2] = Result[3] =  LIGHT_SUM( 1, -1, -1,
                                            1,  0, -1,
                                            1, -1,  0);
        Result[0] = Result[5] =  LIGHT_SUM( 1,  1,  1,
                                            1,  0,  1,
                                            1,  1,  0);
        Result[1] =              LIGHT_SUM( 1,  1, -1,
                                            1,  0, -1,
                                            1,  1,  0);
        Result[4] =              LIGHT_SUM( 1, -1,  1,
                                            1,  0,  1,
                                            1, -1,  0);
        break;
      }
    }
    return Result;
  }

public:
  VoxelMapRenderer(VoxelChunk &Map, v3 offset) : Array("textures.bmp:nearest") {
    setMap(&Map, offset);
  }
  VoxelMapRenderer() : Array("textures.bmp:nearest") {
  }

  static size_t getSize() {
    return 16;
  }

  void setMap(VoxelChunk *M, v3 offset) {
    this->offset = offset;
    this->size = size;
    Map = M;
    recreate();
  }

  void recreate() {
    Array.reset();
    for (int64_t x = offset.x; x < size.x + offset.x; ++x) {
      for (int64_t y = offset.y; y < size.y + offset.y; ++y) {
        for (int64_t z = offset.z; z < size.z + offset.z; ++z) {
          AnnotatedVoxel V = Map->getAnnotated(v3(x, y, z));
          if (V.V.transparent())
            continue;
          const float us = Voxel::TEX_SIZE;
          const float vs = us;

          ADD_VOXEL_SIDE(
            x, y + 1, z,
            x, y + 1, z + 1,
            x + 1, y + 1, z + 1,
            x + 1, y + 1, z,
            0);
          ADD_VOXEL_SIDE(
            x, y, z,
            x, y, z + 1,
            x + 1, y, z + 1,
            x + 1, y, z,
            1);
          ADD_VOXEL_SIDE(
            x, y + 1, z,
            x + 1, y + 1, z,
            x + 1, y, z,
            x, y, z,
            2);
          ADD_VOXEL_SIDE(
            x, y + 1, z + 1,
            x + 1, y + 1, z + 1,
            x + 1, y, z + 1,
            x, y, z + 1,
            3);
          ADD_VOXEL_SIDE(
            x, y + 1, z + 1,
            x, y + 1, z,
            x, y, z,
            x, y, z + 1,
            4);
          ADD_VOXEL_SIDE(
            x + 1, y + 1, z + 1,
            x + 1, y + 1, z,
            x + 1, y, z,
            x + 1, y, z + 1,
            5);
        }
      }
    }
    Array.finalize();
  }

  void draw() {
    Array.draw();
  }

};

#endif // VOXELMAPRENDERER_H
