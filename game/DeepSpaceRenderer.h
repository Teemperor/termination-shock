#ifndef DEEPSPACERENDERER_H
#define DEEPSPACERENDERER_H

#include <random>
#include "v3.h"
#include "StarArray.h"
#include "common/Common.h"

class DeepSpaceRenderer {
  StarArray PlanetArray;
  StarArray StarArray_;

public:
  DeepSpaceRenderer() : PlanetArray("earth.bmp:linear"), StarArray_("star.bmp:linear") {
    const float size = 9000;
    float distance = -13000;
    PlanetArray.add(Rec(v3f(size, -size, distance), v3f(size, size, distance), v3f(-size, size, distance), v3f(-size, -size, distance)), 1.0f);
    PlanetArray.finalize();

    std::default_random_engine generator;
    std::uniform_real_distribution<double> angleDistribution(-PI, PI);
    std::uniform_real_distribution<double> sizeDistribution(0.1f, 1.4f);
    std::uniform_real_distribution<double> distanceDistribution(0.90f, 1.1f);
    std::uniform_real_distribution<double> lightDistribution(0, 1);
    std::uniform_int_distribution<int> textureDistribution(0, 5);

    distance = 18000;

    const float starSize = 180;
    int StarCount = 2700;
    for (int i = 0; i < StarCount; ++i) {
      double horizontalAngle = angleDistribution(generator);
      double verticalAngle = angleDistribution(generator);
      double sizeFactor = sizeDistribution(generator);
      double distanceFactor = distanceDistribution(generator);
      double lightFactor = lightDistribution(generator);

      int u = textureDistribution(generator);
      if (u > 1)
        u = 0;
      int v = textureDistribution(generator);
      if (v > 1)
        v = 0;


      v3f pos = v3f(
        (float) (cos(verticalAngle) * sin(horizontalAngle)),
        (float) sin(verticalAngle),
        (float) (cos(verticalAngle) * cos(horizontalAngle)));

      verticalAngle += PI / 2;

      v3f helperUp = v3f(
        (float) (cos(verticalAngle) * sin(horizontalAngle)),
        (float) sin(verticalAngle),
        (float) (cos(verticalAngle) * cos(horizontalAngle)));

      v3f right = helperUp.cross(pos).normalize();

      v3f up = right.cross(pos).normalize();

      up *= starSize * sizeFactor;
      right *= starSize * sizeFactor;
      pos *= distance * distanceFactor;

      StarArray_.add(Rec(pos + right + up,
                        pos - right + up,
                        pos - right - up,
                        pos + right - up, std::make_pair(u * 0.5f, v * 0.5f), 0.5f, 0.5f
      ), lightFactor);
    }

    StarArray_.finalize();
  }

  void draw() {
    StarArray_.draw();
    PlanetArray.draw();
  }
};

#endif // DEEPSPACERENDERER_H
