#include "Voxel.h"

#include "Map.h"

void Voxel::callback(bool added, const v3 &pos, VoxelChunk &Chunk) {
  switch(Type) {
    case LAMP:
      if (added)
        Chunk.addLight(pos);
      else
        Chunk.removeLight(pos);
      break;
  }
}