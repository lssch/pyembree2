#pragma once

#include <pybind11/numpy.h>

#include <cstddef>
#include <cstdint>

namespace py = pybind11;

class TriangleMesh {
public:
  TriangleMesh(py::array vertices, py::array faces,
               py::array vertex_normals = py::array());

  const py::array_t<float> &vertices() const { return vertices_; }
  const py::array_t<uint32_t> &faces() const { return faces_; }
  const py::array_t<float> &vertex_normals() const { return vertex_normals_; }

  bool has_vertex_normals() const;

private:
  py::array_t<float> vertices_;
  py::array_t<uint32_t> faces_;
  py::array_t<float> vertex_normals_;
};
