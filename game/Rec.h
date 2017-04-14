#ifndef REC_H
#define REC_H

#include <GL/glew.h>
#include <GL/gl.h>
#include <vector>
#include "v3.h"

struct Rec {

  std::vector<GLfloat> vertexes;
  std::vector<GLfloat> uvs;

  Rec(const v3f &d, const v3f &c, const v3f &b, const v3f &a,
      std::pair<float, float> uvStart = {0, 0}, float us = 1, float vs = 1) {

    std::pair<float, float> uvEnd = std::make_pair(uvStart.first + us,
                                                   uvStart.second + vs);
    vertexes = {
      d.x, d.y, d.z,
      c.x, c.y, c.z,
      b.x, b.y, b.z,
      b.x, b.y, b.z,
      a.x, a.y, a.z,
      d.x, d.y, d.z,
    };

    // Two UV coordinatesfor each vertex. They were created withe Blender.
    uvs = {
      uvStart.first, uvStart.second,
      uvEnd.first, uvStart.second,
      uvEnd.first, uvEnd.second,
      uvEnd.first, uvEnd.second,
      uvStart.first, uvEnd.second,
      uvStart.first, uvStart.second,
    };
  }

};


#endif // REC_H
