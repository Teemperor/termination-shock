#ifndef TUTORIALS_V3_H
#define TUTORIALS_V3_H

#include <cmath>
#include <ostream>
#include <cstdint>
#include <tuple>

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

  bool operator>(const v3 &Other) const {
    return std::tie(x, y, z) > std::tie(Other.x, Other.y, Other.z);
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




#endif //TUTORIALS_V3_H
