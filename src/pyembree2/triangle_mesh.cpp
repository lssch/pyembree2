#include "triangle_mesh.h"

/**
 * @brief Validates the arrays of a triangle mesh.
 *
 * @param[in] mesh  Triangle mesh to validate.
 * @param[in] index Index of the mesh in the input collection.
 * @param[in] require_vertex_normals Whether vertex normals are required.
 *
 * @throws std::invalid_argument If any mesh array has an invalid shape,
 *                               layout, or number of rows.
 */
void validate_triangle_mesh(const triangle_mesh &mesh, std::size_t index,
                            bool require_vertex_normals) {
  const auto &vertices = mesh.vertices;
  const auto &faces = mesh.faces;

  if (vertices.ndim() != 2 || vertices.shape(1) != 3) {
    throw std::invalid_argument("mesh " + std::to_string(index) +
                                ": vertices must have shape (N, 3)");
  }

  if ((vertices.flags() & py::array::c_style) == 0) {
    throw std::invalid_argument("mesh " + std::to_string(index) +
                                ": vertices must be C-contiguous");
  }

  if (faces.ndim() != 2 || faces.shape(1) != 3) {
    throw std::invalid_argument("mesh " + std::to_string(index) +
                                ": faces must have shape (N, 3)");
  }

  if ((faces.flags() & py::array::c_style) == 0) {
    throw std::invalid_argument("mesh " + std::to_string(index) +
                                ": faces must be C-contiguous");
  }

  if (require_vertex_normals) {
    if (mesh.vertex_normals.size() == 0) {
      throw std::invalid_argument(
          "mesh " + std::to_string(index) +
          ": vertex_normals must be provided for signed_distance calculation");
    }
    const auto &normals = mesh.vertex_normals;

    if (normals.ndim() != 2 || normals.shape(1) != 3) {
      throw std::invalid_argument("mesh " + std::to_string(index) +
                                  ": vertex_normals must have shape (N, 3)");
    }

    if (normals.shape(0) != vertices.shape(0)) {
      throw std::invalid_argument(
          "mesh " + std::to_string(index) +
          ": vertex_normals must have the same number of rows as vertices");
    }

    if ((normals.flags() & py::array::c_style) == 0) {
      throw std::invalid_argument("mesh " + std::to_string(index) +
                                  ": vertex_normals must be C-contiguous");
    }
  }
}
