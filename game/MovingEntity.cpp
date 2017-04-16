#include "MovingEntity.h"

#include "Map.h"

bool MovingEntity::isHeightGood(float h) {
  static const float r = 0.3f;
  return
      space->get(v3f(pos.x + r, pos.y + h, pos.z - r).toVoxelPos()).isFree() &&
      space->get(v3f(pos.x - r, pos.y + h, pos.z - r).toVoxelPos()).isFree() &&
      space->get(v3f(pos.x + r, pos.y + h, pos.z + r).toVoxelPos()).isFree() &&
      space->get(v3f(pos.x - r, pos.y + h, pos.z + r).toVoxelPos()).isFree();
}

bool MovingEntity::gravityAffected() const {
  return space->isGravityAffected(v3f(pos.x, pos.y - 1.5f, pos.z).toVoxelPos());
}
