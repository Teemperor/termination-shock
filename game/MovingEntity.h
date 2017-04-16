#ifndef MOVINGENTITY_H
#define MOVINGENTITY_H

#include "Entity.h"

class MovingEntity : public Entity {
  Space *space;

  v3f vel;

  bool isHeightGood(float h);

  bool gravityAffected() const;

  bool isPosGood() {
    return isHeightGood(-1.5f) && isHeightGood(-0.8f) && isHeightGood(0) && isHeightGood(0.3f);
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
  MovingEntity(Space *space) : space(space), vel(0, 0, 0) {
    vel.y = -0.4f;
  }

  bool isCollidingWith(v3 p, float h) {
    static const float r = 0.3f;
    return
      v3f(pos.x + r, pos.y + h, pos.z - r).toVoxelPos() == p ||
      v3f(pos.x - r, pos.y + h, pos.z - r).toVoxelPos() == p ||
      v3f(pos.x + r, pos.y + h, pos.z + r).toVoxelPos() == p ||
      v3f(pos.x - r, pos.y + h, pos.z + r).toVoxelPos() == p;
  }

  bool isCollidingWith(v3 pos) {
    return isCollidingWith(pos, -1.5f) || isCollidingWith(pos, -0.8f) || isCollidingWith(pos, 0) || isCollidingWith(pos, 0.3f);
  }

  void setMove(float hRot, float dx, float dy, float dz) {
    const float speed = 4.5;
    rot = hRot;
    float moveRot = rot;
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
#endif // MOVINGENTITY_H
