#pragma once

#include <algorithm>
#include <array>
#include <cmath>

using vec3 = std::array<float, 3>;

inline vec3 vec_sub(const vec3 &a, const vec3 &b) {
  return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}

inline vec3 vec_add(const vec3 &a, const vec3 &b) {
  return {a[0] + b[0], a[1] + b[1], a[2] + b[2]};
}

inline vec3 vec_scale(const vec3 &a, float s) {
  return {a[0] * s, a[1] * s, a[2] * s};
}

inline float vec_dot(const vec3 &a, const vec3 &b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline vec3 vec_cross(const vec3 &a, const vec3 &b) {
  return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2],
          a[0] * b[1] - a[1] * b[0]};
}

// Returns {0, 0, 0} for near-zero-length input rather than dividing by ~0.
inline vec3 vec_normalize(const vec3 &v) {
  const float length = std::sqrt(vec_dot(v, v));
  if (length < 1e-20f) {
    return {0.0f, 0.0f, 0.0f};
  }
  return {v[0] / length, v[1] / length, v[2] / length};
}
