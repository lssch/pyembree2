#include "triangle_mesh.h"

#include <stdexcept>
#include <string>
#include <utility>

TriangleMesh::TriangleMesh(py::array vertices, py::array faces,
                           py::array vertex_normals) {
  // Validate vertices
  if (vertices.ndim() != 2 || vertices.shape(1) != 3) {
    throw std::invalid_argument("vertices must have shape (N, 3)");
  }

  if (!(vertices.flags() & py::array::c_style)) {
    throw std::invalid_argument("vertices must be C-contiguous");
  }

  // Validate faces
  if (faces.ndim() != 2 || faces.shape(1) != 3) {
    throw std::invalid_argument("faces must have shape (N, 3)");
  }

  if (!(faces.flags() & py::array::c_style)) {
    throw std::invalid_argument("faces must be C-contiguous");
  }

  // Validate vertex normals if provided
  if (vertex_normals.size() > 0) {
    if (vertex_normals.ndim() != 2 || vertex_normals.shape(1) != 3) {
      throw std::invalid_argument("vertex_normals must have shape (N, 3)");
    }

    if (!(vertex_normals.flags() & py::array::c_style)) {
      throw std::invalid_argument("vertex_normals must be C-contiguous");
    }
  }

  // Convert to typed arrays without force-casting.
  vertices_ = py::cast<py::array_t<float>>(vertices);
  faces_ = py::cast<py::array_t<uint32_t>>(faces);

  if (vertex_normals.size() > 0) {
    vertex_normals_ = py::cast<py::array_t<float>>(vertex_normals);
  }
}

bool TriangleMesh::has_vertex_normals() const {
  return vertex_normals_.size() != 0;
}
