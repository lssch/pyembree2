
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <embree4/rtcore.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "triangle_mesh.h"
#include "vec3.h"

namespace py = pybind11;

/**
 * @brief Computes the closest point on a triangle to a query point.
 *
 * @param[in]  p     Query point.
 * @param[in]  a     First triangle vertex.
 * @param[in]  b     Second triangle vertex.
 * @param[in]  c     Third triangle vertex.
 * @param[out] q     Closest point on the triangle.
 * @param[out] bary  Barycentric coordinates of @p q with respect to
 *                   @p (a, b, c).
 *
 * @return Squared Euclidean distance between @p p and @p q.
 *
 * The closest point may lie at a vertex, on an edge, or in the
 * triangle interior.
 */
static float closest_point_triangle(const vec3 &p, const vec3 &a, const vec3 &b,
                                    const vec3 &c, vec3 &q, vec3 &bary) {
  const vec3 ab = vec_sub(b, a);
  const vec3 ac = vec_sub(c, a);
  const vec3 ap = vec_sub(p, a);

  const float d1 = vec_dot(ab, ap);
  const float d2 = vec_dot(ac, ap);

  // A
  if (d1 <= 0.0f && d2 <= 0.0f) {
    q = a;
    bary = {1.0f, 0.0f, 0.0f};
    const vec3 d = vec_sub(p, q);
    return vec_dot(d, d);
  }

  const vec3 bp = vec_sub(p, b);
  const float d3 = vec_dot(ab, bp);
  const float d4 = vec_dot(ac, bp);

  // B
  if (d3 >= 0.0f && d4 <= d3) {
    q = b;
    bary = {0.0f, 1.0f, 0.0f};
    const vec3 d = vec_sub(p, q);
    return vec_dot(d, d);
  }

  // AB
  const float vc = d1 * d4 - d3 * d2;
  if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
    const float v = d1 / (d1 - d3);
    q = vec_add(a, vec_scale(ab, v));
    bary = {1.0f - v, v, 0.0f};
    const vec3 d = vec_sub(p, q);
    return vec_dot(d, d);
  }

  const vec3 cp = vec_sub(p, c);
  const float d5 = vec_dot(ab, cp);
  const float d6 = vec_dot(ac, cp);

  // C
  if (d6 >= 0.0f && d5 <= d6) {
    q = c;
    bary = {0.0f, 0.0f, 1.0f};
    const vec3 d = vec_sub(p, q);
    return vec_dot(d, d);
  }

  // AC
  const float vb = d5 * d2 - d1 * d6;
  if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
    const float w = d2 / (d2 - d6);
    q = vec_add(a, vec_scale(ac, w));
    bary = {1.0f - w, 0.0f, w};
    const vec3 d = vec_sub(p, q);
    return vec_dot(d, d);
  }

  // BC
  const float va = d3 * d6 - d5 * d4;
  if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
    const float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
    q = vec_add(b, vec_scale(vec_sub(c, b), w));
    bary = {0.0f, 1.0f - w, w};
    const vec3 d = vec_sub(p, q);
    return vec_dot(d, d);
  }

  // Inside triangle
  const float denom = 1.0f / (va + vb + vc);
  const float v = vb * denom;
  const float w = vc * denom;

  q = vec_add(a, vec_add(vec_scale(ab, v), vec_scale(ac, w)));
  bary = {1.0f - v - w, v, w};

  const vec3 d = vec_sub(p, q);
  return vec_dot(d, d);
}

/**
 * @brief Per-query state shared with the Embree point-query callback.
 */
struct query_context {
  const std::vector<triangle_mesh> *meshes;
  bool compute_sign;
  uint32_t mesh_index;
  uint32_t face_index;
  vec3 closest_point;
  vec3 closest_normal;
  float closest_distance_squared;
};

/**
 * @brief Evaluates one triangle during an Embree point query.
 *
 * The callback computes the exact closest point on the triangle and
 * updates the query state when the triangle is closer than the current
 * best result.
 *
 * When signed distances are requested, the callback also computes the
 * normal at the closest point. Vertex normals are barycentrically
 * interpolated when available; otherwise the flat triangle normal is used.
 *
 * @param[in,out] args Embree point-query callback arguments.
 *
 * @return Always @c true to continue processing the query.
 */
static bool point_query_callback(RTCPointQueryFunctionArguments *args) {
  auto *context = static_cast<query_context *>(args->userPtr);

  const triangle_mesh &mesh = (*context->meshes)[args->geomID];

  const auto vertices = mesh.vertices.unchecked<2>();
  const auto faces = mesh.faces.unchecked<2>();

  const uint32_t primitive_id = args->primID;

  const uint32_t i0 = faces(primitive_id, 0);
  const uint32_t i1 = faces(primitive_id, 1);
  const uint32_t i2 = faces(primitive_id, 2);

  const vec3 a = {vertices(i0, 0), vertices(i0, 1), vertices(i0, 2)};
  const vec3 b = {vertices(i1, 0), vertices(i1, 1), vertices(i1, 2)};
  const vec3 c = {vertices(i2, 0), vertices(i2, 1), vertices(i2, 2)};

  const vec3 p = {args->query->x, args->query->y, args->query->z};

  vec3 closest;
  vec3 bary;

  const float distance_squared =
      closest_point_triangle(p, a, b, c, closest, bary);

  if (distance_squared < context->closest_distance_squared) {
    context->closest_distance_squared = distance_squared;
    context->closest_point = closest;
    /**
     * Embree geometry and primitive IDs map directly to the input
     * mesh and triangle indices because geometries are attached in
     * the same order as the input mesh vector.
     */
    context->mesh_index = args->geomID;
    context->face_index = args->primID;

    if (context->compute_sign) {
      // Interpolate vertex normals using the barycentric coordinates of the
      // closest point.
      const auto normals = mesh.vertex_normals.unchecked<2>();
      const vec3 na = {normals(i0, 0), normals(i0, 1), normals(i0, 2)};
      const vec3 nb = {normals(i1, 0), normals(i1, 1), normals(i1, 2)};
      const vec3 nc = {normals(i2, 0), normals(i2, 1), normals(i2, 2)};

      context->closest_normal =
          vec_add(vec_scale(na, bary[0]),
                  vec_add(vec_scale(nb, bary[1]), vec_scale(nc, bary[2])));
    }

    // Restrict subsequent callback invocations to triangles that may improve
    // the current closest-point result.
    args->query->radius = std::sqrt(distance_squared);
  }

  return true;
}

/**
 * @brief Executes an unsigned or signed point-to-mesh distance query.
 *
 * This function constructs an Embree scene from the supplied triangle
 * meshes, performs one point query per input point, and collects the
 * closest mesh, triangle, point, and distance.
 *
 * @param[in] meshes       Triangle meshes to query.
 * @param[in] points       Query points with shape @c (N, 3).
 * @param[in] compute_sign If true, compute signed distances using the
 *                         normal at each closest point.
 *
 * @return A tuple containing:
 *         - mesh index for each query point,
 *         - triangle index for each query point,
 *         - closest point on the corresponding triangle,
 *         - unsigned or signed distance to that point.
 *
 * @throws std::invalid_argument If mesh or point arrays have invalid
 *         dimensions, incompatible sizes, are not C-contiguous or
 *         have invalid normals.
 * @throws std::runtime_error If the Embree device, scene, or geometry
 *         cannot be created.
 */
static std::tuple<py::array_t<uint32_t>, py::array_t<uint32_t>,
                  py::array_t<double>, py::array_t<double>>
run_distance_query(const std::vector<triangle_mesh> &meshes,
                   py::array_t<double> points, bool compute_sign) {

  // Validate meshes
  for (ssize_t i = 0; i < meshes.size(); ++i) {
    const auto &mesh = meshes[i];
    auto vertices = mesh.vertices.request();
    auto faces = mesh.faces.request();

    if (vertices.ndim != 2 || vertices.shape[1] != 3) {
      throw std::invalid_argument(
          std::format("mesh {}: vertices must have shape (N, 3)", i));
    }

    if (!(mesh.vertices.flags() & py::array::c_style)) {
      throw std::invalid_argument(
          std::format("mesh {}: vertices must be C-contiguous", i));
    }

    if (faces.ndim != 2 || faces.shape[1] != 3) {
      throw std::invalid_argument(
          std::format("mesh {}: faces must have shape (N, 3)", i));
    }

    if (!(mesh.faces.flags() & py::array::c_style)) {
      throw std::invalid_argument(
          std::format("mesh {}: faces must be C-contiguous", i));
    }

    if (compute_sign) {
      if (mesh.vertex_normals.size() == 0) {
        throw std::invalid_argument(
            std::format("mesh {}: vertex_normals must be provided for "
                        "signed_distance calculation",
                        i));
      }

      auto normals = mesh.vertex_normals.request();

      if (normals.ndim != 2 || normals.shape[1] != 3) {
        throw std::invalid_argument(
            std::format("mesh {}: vertex_normals must have shape (N, 3)", i));
      }

      if (normals.shape[0] != vertices.shape[0]) {
        throw std::invalid_argument(
            std::format("mesh {}: vertex_normals must have the same number of "
                        "rows as vertices",
                        i));
      }

      if (!(mesh.vertex_normals.flags() & py::array::c_style)) {
        throw std::invalid_argument(
            std::format("mesh {}: vertex_normals must be C-contiguous", i));
      }
    }
  }

  // Validate points
  auto points_info = points.request();
  if (points_info.ndim != 2 || points_info.shape[1] != 3) {
    throw std::invalid_argument("points must have shape (N, 3)");
  }

  const ssize_t num_points = points_info.shape[0];

  // Create Embree device and scene
  RTCDevice device = rtcNewDevice(nullptr);
  if (!device) {
    throw std::runtime_error("Failed to create Embree device");
  }

  RTCScene scene = rtcNewScene(device);
  if (!scene) {
    rtcReleaseDevice(device);
    throw std::runtime_error("Failed to create Embree scene");
  }

  // Add meshes to scene
  for (const auto &mesh : meshes) {
    auto vertices = mesh.vertices.request();
    auto faces = mesh.faces.request();

    RTCGeometry geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

    if (!geometry) {
      rtcReleaseScene(scene);
      rtcReleaseDevice(device);
      throw std::runtime_error("Failed to create Embree geometry");
    }

    // Vertex buffer
    rtcSetSharedGeometryBuffer(
        geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, vertices.ptr, 0,
        sizeof(float) * 3, static_cast<size_t>(vertices.shape[0]));

    // Index buffer
    rtcSetSharedGeometryBuffer(
        geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, faces.ptr, 0,
        sizeof(uint32_t) * 3, static_cast<size_t>(faces.shape[0]));

    rtcSetGeometryPointQueryFunction(geometry, point_query_callback);
    rtcCommitGeometry(geometry);
    // Geometries are attached in input-vector order, so Embree's
    // geometry ID corresponds to the mesh index used in the results.
    rtcAttachGeometry(scene, geometry);
    rtcReleaseGeometry(geometry);
  }

  rtcCommitScene(scene);

  py::array_t<double> closest_points({num_points, static_cast<ssize_t>(3)});
  py::array_t<uint32_t> mesh_indices(num_points);
  py::array_t<double> distances(num_points);
  py::array_t<uint32_t> triangle_indices(num_points);

  auto closest_info = closest_points.request();
  auto mesh_info = mesh_indices.request();
  auto distance_info = distances.request();
  auto triangle_info = triangle_indices.request();

  auto *closest_ptr = static_cast<double *>(closest_info.ptr);
  auto *mesh_ptr = static_cast<uint32_t *>(mesh_info.ptr);
  auto *distance_ptr = static_cast<double *>(distance_info.ptr);
  auto *triangle_ptr = static_cast<uint32_t *>(triangle_info.ptr);

  const auto *point_ptr = static_cast<const double *>(points_info.ptr);

  // Perform queries
  RTCPointQueryContext context;
  rtcInitPointQueryContext(&context);

  for (ssize_t i = 0; i < num_points; ++i) {
    const double *p_in = point_ptr + i * 3;
    const vec3 p = {static_cast<float>(p_in[0]), static_cast<float>(p_in[1]),
                    static_cast<float>(p_in[2])};

    RTCPointQuery query;
    query.x = p[0];
    query.y = p[1];
    query.z = p[2];
    // Start with an unbounded search radius; the callback tightens it after
    // finding a closer triangle.
    query.radius = std::numeric_limits<float>::infinity();
    query.time = 0.0f;

    // Per-point state passed to the Embree callback.
    // The sentinel indices indicate that no triangle has been found.
    query_context query_data;
    query_data.meshes = &meshes;
    query_data.compute_sign = compute_sign;
    query_data.closest_point = vec3{0.0f, 0.0f, 0.0f};
    query_data.closest_normal = vec3{0.0f, 0.0f, 0.0f};
    query_data.mesh_index = std::numeric_limits<uint32_t>::max();
    query_data.face_index = std::numeric_limits<uint32_t>::max();
    query_data.closest_distance_squared =
        std::numeric_limits<float>::infinity();

    rtcPointQuery(scene, &query, &context, nullptr, &query_data);

    closest_ptr[i * 3 + 0] = static_cast<double>(query_data.closest_point[0]);
    closest_ptr[i * 3 + 1] = static_cast<double>(query_data.closest_point[1]);
    closest_ptr[i * 3 + 2] = static_cast<double>(query_data.closest_point[2]);
    mesh_ptr[i] = query_data.mesh_index;
    triangle_ptr[i] = query_data.face_index;

    const double unsigned_distance =
        std::sqrt(static_cast<double>(query_data.closest_distance_squared));

    if (compute_sign) {
      const vec3 diff = vec_sub(p, query_data.closest_point);
      const float dot = vec_dot(diff, query_data.closest_normal);
      distance_ptr[i] = (dot < 0.0f) ? -unsigned_distance : unsigned_distance;
    } else {
      distance_ptr[i] = unsigned_distance;
    }
  }

  rtcReleaseScene(scene);
  rtcReleaseDevice(device);

  return std::make_tuple(std::move(mesh_indices), std::move(triangle_indices),
                         std::move(closest_points), std::move(distances));
}

/**
 * @brief Computes the unsigned distance from points to the closest
 *        triangle in a collection of meshes.
 *
 * @param[in] meshes  Triangle meshes to query.
 * @param[in] points  Query points with shape @c (N, 3).
 *
 * @return A tuple containing:
 *         - the closest mesh index for each point,
 *         - the closest triangle index for each point,
 *         - the closest points on the meshes,
 *         - the corresponding unsigned distances.
 */
std::tuple<py::array_t<uint32_t>, py::array_t<uint32_t>, py::array_t<double>,
           py::array_t<double>>
unsigned_distance(std::vector<triangle_mesh> meshes,
                  py::array_t<double> points) {
  return run_distance_query(meshes, points, false);
}

/**
 * @brief Computes the signed distance from points to the closest
 *        triangle in a collection of meshes.
 *
 * @param[in] meshes  Triangle meshes to query.
 * @param[in] points  Query points with shape @c (N, 3).
 *
 * @return A tuple containing:
 *         - the closest mesh index for each point,
 *         - the closest triangle index for each point,
 *         - the closest points on the meshes,
 *         - the corresponding signed distances.
 *
 * @note If a mesh provides @c vertex_normals, the normal at the closest
 *       point is computed by barycentric interpolation of those normals.
 *       Otherwise, the flat normal of the closest triangle is used.
 *       Flat normals may produce incorrect sign classification near sharp
 *       edges, vertices, or concave features.
 */
std::tuple<py::array_t<uint32_t>, py::array_t<uint32_t>, py::array_t<double>,
           py::array_t<double>>
signed_distance(std::vector<triangle_mesh> meshes, py::array_t<double> points) {
  return run_distance_query(meshes, points, true);
}

PYBIND11_MODULE(pyembree2, m, py::mod_gil_not_used()) {
  m.doc() = "Point-to-triangle-mesh distance and signed-distance queries "
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
}
