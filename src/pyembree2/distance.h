#pragma once

#include <cstdint>
#include <tuple>
#include <vector>

#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "triangle_mesh.h"

namespace py = pybind11;

std::tuple<py::array_t<uint32_t>, py::array_t<uint32_t>, py::array_t<double>,
           py::array_t<double>>
unsigned_distance(std::vector<triangle_mesh> meshes,
                  py::array_t<double> points);

std::tuple<py::array_t<uint32_t>, py::array_t<uint32_t>, py::array_t<double>,
           py::array_t<double>>
signed_distance(std::vector<triangle_mesh> meshes, py::array_t<double> points);
