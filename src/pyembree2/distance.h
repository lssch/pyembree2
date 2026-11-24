#pragma once

#include <cstdint>
#include <tuple>
#include <vector>

#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "scene.h"

namespace py = pybind11;

bool point_query_callback(RTCPointQueryFunctionArguments *args);

std::tuple<py::array_t<uint32_t>, py::array_t<uint32_t>, py::array_t<double>,
           py::array_t<double>>
unsigned_distance(const Scene &scene, py::array_t<double> points);

std::tuple<py::array_t<uint32_t>, py::array_t<uint32_t>, py::array_t<double>,
           py::array_t<double>>
signed_distance(const Scene &scene, py::array_t<double> points);
