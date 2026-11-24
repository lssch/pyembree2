#include <cstddef>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <embree4/rtcore.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "ray_tracing.h"
#include "scene.h"

namespace py = pybind11;

/**
 * @brief Traces rays against a collection of triangle meshes.
 *
 * @param[in] meshes      Triangle meshes to intersect.
 * @param[in] origins     Ray origins with shape (N, 3).
 * @param[in] directions  Ray directions with shape (N, 3).
 *
 * @return A tuple containing:
 *         - mesh indices with shape (N,);
 *         - triangle indices with shape (N,);
 *         - hit distances with shape (N,);
 *         - barycentric @p u coordinates with shape (N,);
 *         - barycentric @p v coordinates with shape (N,).
 *
 * For rays that do not hit a mesh, the mesh and triangle indices are
 * set to UINT32_MAX, and the distance, @p u, and @p v values are set
 * to NaN.
 *
 * The returned distance is the Embree ray parameter @p t. Therefore,
 * it corresponds to Euclidean distance when the input ray directions
 * are normalized.
 */
std::tuple<py::array_t<uint32_t>, py::array_t<uint32_t>, py::array_t<double>,
           py::array_t<double>, py::array_t<double>>
ray_trace(const Scene &scene, py::array_t<double> origins,
          py::array_t<double> directions) {

  // Validate origins
  auto origins_info = origins.request();

  if (origins_info.ndim != 2 || origins_info.shape[1] != 3) {
    throw std::invalid_argument("origins must have shape (N, 3)");
  }

  if (!(origins.flags() & py::array::c_style)) {
    throw std::invalid_argument("origins must be C-contiguous");
  }

  // Validate directions
  auto directions_info = directions.request();

  if (directions_info.ndim != 2 || directions_info.shape[1] != 3) {
    throw std::invalid_argument("directions must have shape (N, 3)");
  }

  if (!(directions.flags() & py::array::c_style)) {
    throw std::invalid_argument("directions must be C-contiguous");
  }

  if (origins_info.shape[0] != directions_info.shape[0]) {
    throw std::invalid_argument(
        "origins and directions must have the same number of rows");
  }

  const ssize_t num_rays = origins_info.shape[0];

  // Allocate output arrays
  py::array_t<uint32_t> mesh_indices(num_rays);
  py::array_t<uint32_t> triangle_indices(num_rays);
  py::array_t<double> distances(num_rays);
  py::array_t<double> u(num_rays);
  py::array_t<double> v(num_rays);

  auto mesh_info = mesh_indices.request();
  auto triangle_info = triangle_indices.request();
  auto distance_info = distances.request();
  auto u_info = u.request();
  auto v_info = v.request();

  auto *mesh_ptr = static_cast<uint32_t *>(mesh_info.ptr);

  auto *triangle_ptr = static_cast<uint32_t *>(triangle_info.ptr);

  auto *distance_ptr = static_cast<double *>(distance_info.ptr);

  auto *u_ptr = static_cast<double *>(u_info.ptr);

  auto *v_ptr = static_cast<double *>(v_info.ptr);

  const auto *origin_ptr = static_cast<const double *>(origins_info.ptr);

  const auto *direction_ptr = static_cast<const double *>(directions_info.ptr);

  // Trace rays
  for (std::size_t i = 0; i < static_cast<std::size_t>(num_rays); ++i) {
    RTCRayHit ray_hit{};

    ray_hit.ray.org_x = origin_ptr[3 * i + 0];
    ray_hit.ray.org_y = origin_ptr[3 * i + 1];
    ray_hit.ray.org_z = origin_ptr[3 * i + 2];

    ray_hit.ray.dir_x = direction_ptr[3 * i + 0];
    ray_hit.ray.dir_y = direction_ptr[3 * i + 1];
    ray_hit.ray.dir_z = direction_ptr[3 * i + 2];

    ray_hit.ray.tnear = 0.0f;
    ray_hit.ray.tfar = std::numeric_limits<float>::infinity();
    ray_hit.ray.time = 0.0f;
    ray_hit.ray.mask = 0xFFFFFFFF;
    ray_hit.ray.id = 0;
    ray_hit.ray.flags = 0;

    ray_hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;

    rtcIntersect1(scene.scene(), &ray_hit, nullptr);

    if (ray_hit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
      mesh_ptr[i] = -1;
      triangle_ptr[i] = -1;
      distance_ptr[i] = std::numeric_limits<double>::quiet_NaN();
      u_ptr[i] = std::numeric_limits<double>::quiet_NaN();
      v_ptr[i] = std::numeric_limits<double>::quiet_NaN();
      continue;
    }

    mesh_ptr[i] = ray_hit.hit.geomID;
    triangle_ptr[i] = ray_hit.hit.primID;
    distance_ptr[i] = static_cast<double>(ray_hit.ray.tfar);
    u_ptr[i] = static_cast<double>(ray_hit.hit.u);
    v_ptr[i] = static_cast<double>(ray_hit.hit.v);
  }

  return std::make_tuple(std::move(mesh_indices), std::move(triangle_indices),
                         std::move(distances), std::move(u), std::move(v));
}
