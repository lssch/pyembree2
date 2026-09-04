#pragma once

#include <cstdint>
#include <tuple>
#include <vector>

#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "triangle_mesh.h"

namespace py = pybind11;

std::tuple<py::array_t<uint32_t>, py::array_t<uint32_t>, py::array_t<double>,
           py::array_t<double>, py::array_t<double>>
ray_trace(const std::vector<triangle_mesh> &meshes, py::array_t<double> origins,
          py::array_t<double> directions);
