#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

struct triangle_mesh {
  py::array_t<float> vertices;
  py::array_t<uint32_t> faces;
};
