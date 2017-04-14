#ifndef VOXELRENDERMAP_H
#define VOXELRENDERMAP_H

#include "VoxelMapRenderer.h"
#include <cstdint>
#include <cstddef>
#include "Voxel.h"
#include <vector>

class VoxelRenderMap {

  std::vector<VoxelMapRenderer *> Renders;
  VoxelChunk *Chunk;

public:
  VoxelRenderMap(VoxelChunk& Chunk) : Chunk(&Chunk) {
    const size_t renderSize = VoxelMapRenderer::getSize();
    for (int64_t x = Chunk.getOffset().x; x < Chunk.getOffset().x + Chunk.getSize().x; x += renderSize)
      for (int64_t y = Chunk.getOffset().y; y < Chunk.getOffset().y + Chunk.getSize().y; y += renderSize)
        for (int64_t z = Chunk.getOffset().z; z < Chunk.getOffset().z + Chunk.getSize().z; z += renderSize)
          Renders.push_back(new VoxelMapRenderer(Chunk, v3(x, y, z)));
  }

  VoxelMapRenderer *get(v3 pos) {
    pos -= Chunk->getOffset();
    size_t rs = Chunk->getSize().x / VoxelMapRenderer::getSize();

    size_t index = (pos.x / VoxelMapRenderer::getSize()) * rs * rs + (pos.y / VoxelMapRenderer::getSize()) * rs + (pos.z / VoxelMapRenderer::getSize());
    if (index < Renders.size())
      return Renders[index];
    return nullptr;
  }

  void recreateSurrounding(v3 pos) {
    static const std::array<v3, 7> Offsets = {
      v3(0, 0, 0),
      v3(0, 0, 1),
      v3(0, 0, -1),
      v3(0, 1, 0),
      v3(0, -1, 0),
      v3(1, 0, 0),
      v3(-1, 0, 0)
    };

    for (int x = -1; x <= 1; ++x)
      for (int y = -1; y <= 1; ++y)
        for (int z = -1; z <= 1; ++z)
          if (auto R = get(v3(pos.x + x * VoxelMapRenderer::getSize(),
                           pos.y + y * VoxelMapRenderer::getSize(),
                           pos.z + z * VoxelMapRenderer::getSize()))) {
            R->recreate();
          }
  }

  void draw() {
    for (auto &R : Renders)
      R->draw();
  }
};

#endif // VOXELRENDERMAP_H
