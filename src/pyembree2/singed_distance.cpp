#include <embree4/rtcore.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "triangle_mesh.h"

namespace py = pybind11;

// -----------------------------------------------------------------------------
// Closest point on triangle
// -----------------------------------------------------------------------------

static float closest_point_triangle(const float p[3], const float a[3],
                                    const float b[3], const float c[3],
                                    float q[3]) {
  const float ab[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
  const float ac[3] = {c[0] - a[0], c[1] - a[1], c[2] - a[2]};
  const float ap[3] = {p[0] - a[0], p[1] - a[1], p[2] - a[2]};
  const float d1 = ab[0] * ap[0] + ab[1] * ap[1] + ab[2] * ap[2];
  const float d2 = ac[0] * ap[0] + ac[1] * ap[1] + ac[2] * ap[2];

  // A
  if (d1 <= 0.0f && d2 <= 0.0f) {
    q[0] = a[0];
    q[1] = a[1];
    q[2] = a[2];

    return (p[0] - q[0]) * (p[0] - q[0]) + (p[1] - q[1]) * (p[1] - q[1]) +
           (p[2] - q[2]) * (p[2] - q[2]);
  }

  const float bp[3] = {p[0] - b[0], p[1] - b[1], p[2] - b[2]};

  const float d3 = ab[0] * bp[0] + ab[1] * bp[1] + ab[2] * bp[2];

  const float d4 = ac[0] * bp[0] + ac[1] * bp[1] + ac[2] * bp[2];

  // B
  if (d3 >= 0.0f && d4 <= d3) {
    q[0] = b[0];
    q[1] = b[1];
    q[2] = b[2];

    return (p[0] - q[0]) * (p[0] - q[0]) + (p[1] - q[1]) * (p[1] - q[1]) +
           (p[2] - q[2]) * (p[2] - q[2]);
  }

  // AB
  const float vc = d1 * d4 - d3 * d2;

  if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
    const float v = d1 / (d1 - d3);

    q[0] = a[0] + v * ab[0];
    q[1] = a[1] + v * ab[1];
    q[2] = a[2] + v * ab[2];

    return (p[0] - q[0]) * (p[0] - q[0]) + (p[1] - q[1]) * (p[1] - q[1]) +
           (p[2] - q[2]) * (p[2] - q[2]);
  }

  const float cp[3] = {p[0] - c[0], p[1] - c[1], p[2] - c[2]};

  const float d5 = ab[0] * cp[0] + ab[1] * cp[1] + ab[2] * cp[2];

  const float d6 = ac[0] * cp[0] + ac[1] * cp[1] + ac[2] * cp[2];

  // C
  if (d6 >= 0.0f && d5 <= d6) {
    q[0] = c[0];
    q[1] = c[1];
    q[2] = c[2];

    return (p[0] - q[0]) * (p[0] - q[0]) + (p[1] - q[1]) * (p[1] - q[1]) +
           (p[2] - q[2]) * (p[2] - q[2]);
  }

  // AC
  const float vb = d5 * d2 - d1 * d6;

  if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
    const float w = d2 / (d2 - d6);

    q[0] = a[0] + w * ac[0];
    q[1] = a[1] + w * ac[1];
    q[2] = a[2] + w * ac[2];

    return (p[0] - q[0]) * (p[0] - q[0]) + (p[1] - q[1]) * (p[1] - q[1]) +
           (p[2] - q[2]) * (p[2] - q[2]);
  }

  // BC
  const float va = d3 * d6 - d5 * d4;

  if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
    const float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));

    q[0] = b[0] + w * (c[0] - b[0]);
    q[1] = b[1] + w * (c[1] - b[1]);
    q[2] = b[2] + w * (c[2] - b[2]);

    return (p[0] - q[0]) * (p[0] - q[0]) + (p[1] - q[1]) * (p[1] - q[1]) +
           (p[2] - q[2]) * (p[2] - q[2]);
  }

  // Inside triangle
  const float denom = 1.0f / (va + vb + vc);

  const float v = vb * denom;
  const float w = vc * denom;

  q[0] = a[0] + ab[0] * v + ac[0] * w;
  q[1] = a[1] + ab[1] * v + ac[1] * w;
  q[2] = a[2] + ab[2] * v + ac[2] * w;

  return (p[0] - q[0]) * (p[0] - q[0]) + (p[1] - q[1]) * (p[1] - q[1]) +
         (p[2] - q[2]) * (p[2] - q[2]);
}

// -----------------------------------------------------------------------------
// Point-query callback
// -----------------------------------------------------------------------------

struct query_context {
  const std::vector<triangle_mesh> *meshes;

  float closest_point[3];

  uint32_t mesh_index;
  uint32_t face_index;

  float closest_distance_squared;
};

static bool point_query_callback(RTCPointQueryFunctionArguments *args) {
  auto *context = static_cast<query_context *>(args->userPtr);

  const triangle_mesh &mesh = (*context->meshes)[args->geomID];

  const auto vertices = mesh.vertices.unchecked<2>();
  const auto faces = mesh.faces.unchecked<2>();

  const uint32_t primitive_id = args->primID;

  const uint32_t i0 = faces(primitive_id, 0);
  const uint32_t i1 = faces(primitive_id, 1);
  const uint32_t i2 = faces(primitive_id, 2);

  const float a[3] = {vertices(i0, 0), vertices(i0, 1), vertices(i0, 2)};

  const float b[3] = {vertices(i1, 0), vertices(i1, 1), vertices(i1, 2)};

  const float c[3] = {vertices(i2, 0), vertices(i2, 1), vertices(i2, 2)};

  const float p[3] = {args->query->x, args->query->y, args->query->z};

  float closest[3];

  const float distance_squared = closest_point_triangle(p, a, b, c, closest);

  if (distance_squared < context->closest_distance_squared) {

    context->closest_distance_squared = distance_squared;

    context->closest_point[0] = closest[0];
    context->closest_point[1] = closest[1];
    context->closest_point[2] = closest[2];

    // Embree IDs:
    //
    // geomID -> mesh index
    // primID -> triangle/face index
    //
    context->mesh_index = args->geomID;
    context->face_index = args->primID;

    // Tighten the search radius.
    args->query->radius = std::sqrt(distance_squared);
  }

  return true;
}

// -----------------------------------------------------------------------------
// Distance query
// -----------------------------------------------------------------------------

std::tuple<py::array_t<double>,   // closest points
           py::array_t<uint32_t>, // mesh IDs
           py::array_t<double>,   // distances
           py::array_t<uint32_t>  // triangle IDs
           >
distance(std::vector<triangle_mesh> meshes, py::array_t<double> points) {

  // ---------------------------------------------------------------------------
  // Validate meshes
  // ---------------------------------------------------------------------------

  for (const auto &mesh : meshes) {
    auto vertices = mesh.vertices.request();
    auto faces = mesh.faces.request();

    if (vertices.ndim != 2 || vertices.shape[1] != 3) {
      throw std::invalid_argument("mesh vertices must have shape (N, 3)");
    }

    if (faces.ndim != 2 || faces.shape[1] != 3) {
      throw std::invalid_argument("mesh faces must have shape (M, 3)");
    }

    if (!(mesh.vertices.flags() & py::array::c_style)) {
      throw std::invalid_argument("mesh vertices must be C-contiguous");
    }

    if (!(mesh.faces.flags() & py::array::c_style)) {
      throw std::invalid_argument("mesh faces must be C-contiguous");
    }
  }

  // ---------------------------------------------------------------------------
  // Validate points
  // ---------------------------------------------------------------------------

  auto points_info = points.request();

  if (points_info.ndim != 2 || points_info.shape[1] != 3) {
    throw std::invalid_argument("points must have shape (N, 3)");
  }

  const ssize_t num_points = points_info.shape[0];

  // ---------------------------------------------------------------------------
  // Create Embree device and scene
  // ---------------------------------------------------------------------------

  RTCDevice device = rtcNewDevice(nullptr);

  if (!device) {
    throw std::runtime_error("Failed to create Embree device");
  }

  RTCScene scene = rtcNewScene(device);

  if (!scene) {
    rtcReleaseDevice(device);

    throw std::runtime_error("Failed to create Embree scene");
  }

  // ---------------------------------------------------------------------------
  // Add meshes to scene
  // ---------------------------------------------------------------------------

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

    // Important: meshes are attached in vector order.
    // Therefore geomID == mesh index.
    rtcAttachGeometry(scene, geometry);

    rtcReleaseGeometry(geometry);
  }

  rtcCommitScene(scene);

  // ---------------------------------------------------------------------------
  // Allocate outputs
  // ---------------------------------------------------------------------------

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

  // ---------------------------------------------------------------------------
  // Perform queries
  // ---------------------------------------------------------------------------

  RTCPointQueryContext context;
  rtcInitPointQueryContext(&context);

  for (ssize_t i = 0; i < num_points; ++i) {

    const double *p = point_ptr + i * 3;

    RTCPointQuery query;

    query.x = static_cast<float>(p[0]);
    query.y = static_cast<float>(p[1]);
    query.z = static_cast<float>(p[2]);

    query.radius = std::numeric_limits<float>::infinity();

    query.time = 0.0f;

    // Per-point result
    query_context query_data;

    query_data.meshes = &meshes;

    query_data.closest_point[0] = 0.0f;
    query_data.closest_point[1] = 0.0f;
    query_data.closest_point[2] = 0.0f;

    query_data.mesh_index = std::numeric_limits<uint32_t>::max();

    query_data.face_index = std::numeric_limits<uint32_t>::max();

    query_data.closest_distance_squared =
        std::numeric_limits<float>::infinity();

    rtcPointQuery(scene, &query, &context, nullptr, &query_data);

    // -------------------------------------------------------------------------
    // Store result
    // -------------------------------------------------------------------------

    closest_ptr[i * 3 + 0] = static_cast<double>(query_data.closest_point[0]);

    closest_ptr[i * 3 + 1] = static_cast<double>(query_data.closest_point[1]);

    closest_ptr[i * 3 + 2] = static_cast<double>(query_data.closest_point[2]);

    mesh_ptr[i] = query_data.mesh_index;

    distance_ptr[i] =
        std::sqrt(static_cast<double>(query_data.closest_distance_squared));

    triangle_ptr[i] = query_data.face_index;
  }

  // ---------------------------------------------------------------------------
  // Cleanup
  // ---------------------------------------------------------------------------

  rtcReleaseScene(scene);
  rtcReleaseDevice(device);

  return std::make_tuple(std::move(closest_points), std::move(mesh_indices),
                         std::move(distances), std::move(triangle_indices));
}

PYBIND11_MODULE(pyembree2, m, py::mod_gil_not_used()) {
  m.doc() = "Point-to-triangle-mesh distance queries backed by Embree4";

  py::class_<triangle_mesh>(m, "triangle_mesh")
      .def(py::init<>())
      .def(py::init([](py::array_t<float> vertices, py::array_t<int> faces) {
             triangle_mesh t;
             t.vertices = vertices;
             t.faces = faces;
             return t;
           }),
           py::arg("vertices"), py::arg("faces"))
      .def_readwrite("vertices", &triangle_mesh::vertices)
      .def_readwrite("faces", &triangle_mesh::faces);

  m.def("distance", &distance, "Distance query with embree (dummy stub)",
        py::arg("meshes"), py::arg("points"));
}
