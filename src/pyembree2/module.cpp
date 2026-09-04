#include <pybind11/pybind11.h>

#include "distance.h"
#include "ray_tracing.h"
#include "triangle_mesh.h"

namespace py = pybind11;

PYBIND11_MODULE(pyembree2, m, py::mod_gil_not_used()) {
  m.doc() = "Point-to-triangle-mesh distance and ray-tracing queries "
            "backed by Embree4";

  py::class_<triangle_mesh>(m, "triangle_mesh")
      .def(py::init<>())
      .def(py::init([](py::array_t<float> vertices, py::array_t<uint32_t> faces,
                       py::array_t<float> vertex_normals) {
             triangle_mesh t;
             t.vertices = vertices;
             t.faces = faces;
             t.vertex_normals = vertex_normals;
             return t;
           }),
           py::arg("vertices"), py::arg("faces"),
           py::arg("vertex_normals") = py::array_t<float>())
      .def_readwrite("vertices", &triangle_mesh::vertices)
      .def_readwrite("faces", &triangle_mesh::faces)
      .def_readwrite("vertex_normals", &triangle_mesh::vertex_normals);

  m.def("unsigned_distance", &unsigned_distance, py::arg("meshes"),
        py::arg("points"));

  m.def("signed_distance", &signed_distance, py::arg("meshes"),
        py::arg("points"));

  m.def("ray_trace", &ray_trace, py::arg("meshes"), py::arg("origins"),
        py::arg("directions"));
}
