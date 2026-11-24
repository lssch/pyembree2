#pragma once

#include "triangle_mesh.h"
#include <embree4/rtcore.h>
#include <vector>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

class Scene {
public:
  Scene();
  ~Scene();

  const std::vector<TriangleMesh> &meshes() const { return meshes_; }
  const std::vector<unsigned int> &geometry_ids() const {
    return geometry_ids_;
  }

  const RTCScene &scene() const { return scene_; }

  void add_mesh(const TriangleMesh &mesh);

private:
  RTCDevice device_ = nullptr;
  RTCScene scene_ = nullptr;
  std::vector<TriangleMesh> meshes_;
  std::vector<unsigned int> geometry_ids_;

  void commit();
};
