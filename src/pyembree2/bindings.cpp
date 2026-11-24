#include <pybind11/pybind11.h>

#include "distance.h"
#include "ray_tracing.h"
#include "scene.h"
#include "triangle_mesh.h"

namespace py = pybind11;

PYBIND11_MODULE(pyembree2, m, py::mod_gil_not_used()) {
  m.doc() = "Point-to-triangle-mesh distance and ray-tracing queries "
            "backed by Embree4";

  py::class_<TriangleMesh>(m, "TriangleMesh")
      .def(py::init<py::array, py::array, py::array>(), py::arg("vertices"),
           py::arg("faces"), py::arg("vertex_normals") = py::array_t<float>())
      .def_property_readonly("vertices", &TriangleMesh::vertices)
      .def_property_readonly("faces", &TriangleMesh::faces)
      .def_property_readonly("vertex_normals", &TriangleMesh::vertex_normals)
      .def_property_readonly("has_vertex_normals",
                             &TriangleMesh::has_vertex_normals);

  py::class_<Scene>(m, "Scene")
      .def(py::init<>())
      .def("add_mesh", &Scene::add_mesh, py::arg("mesh"));

  m.def("unsigned_distance", &unsigned_distance, py::arg("scene"),
        py::arg("points"));

  m.def("signed_distance", &signed_distance, py::arg("scene"),
        py::arg("points"));

  m.def("ray_trace", &ray_trace, py::arg("scene"), py::arg("origins"),
        py::arg("directions"));
}
