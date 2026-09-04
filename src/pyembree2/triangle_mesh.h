#pragma once

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

struct triangle_mesh {
  py::array_t<float> vertices;
  py::array_t<uint32_t> faces;
  py::array_t<float> vertex_normals;
};

void validate_triangle_mesh(const triangle_mesh &mesh, std::size_t index,
                            bool require_vertex_normals = false);
