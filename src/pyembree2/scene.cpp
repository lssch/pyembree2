#include "scene.h"
#include "distance.h"
#include "triangle_mesh.h"
#include <ranges>
#include <utility>
#include <vector>

Scene::Scene() {
  device_ = rtcNewDevice(nullptr);
  if (!device_) {
    throw std::runtime_error("Failed to create Embree device");
  }

  scene_ = rtcNewScene(device_);
  if (!scene_) {
    rtcReleaseDevice(device_);
    throw std::runtime_error("Failed to create Embree scene");
  }
}

Scene::~Scene() {
  if (scene_) {
    rtcReleaseScene(scene_);
  }

  if (device_) {
    rtcReleaseDevice(device_);
  }
}

void Scene::add_mesh(const TriangleMesh &mesh) {
  auto vertices = mesh.vertices().request();
  auto faces = mesh.faces().request();

  RTCGeometry geometry = rtcNewGeometry(device_, RTC_GEOMETRY_TYPE_TRIANGLE);
  if (!geometry) {
    throw std::runtime_error("Failed to create Embree geometry");
  }

  // Vertex buffer
  rtcSetSharedGeometryBuffer(
      geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, vertices.ptr, 0,
      sizeof(float) * 3, static_cast<size_t>(vertices.shape[0]));

  // Faces buffer
  rtcSetSharedGeometryBuffer(
      geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, faces.ptr, 0,
      sizeof(uint32_t) * 3, static_cast<size_t>(faces.shape[0]));

  rtcCommitGeometry(geometry);
  rtcSetGeometryPointQueryFunction(geometry, point_query_callback);
  // Geometries are attached in input-vector order, so Embree's
  // geometry ID corresponds to the mesh index used in the results.
  unsigned int geomety_id = rtcAttachGeometry(scene_, geometry);
  rtcReleaseGeometry(geometry);

  if (geomety_id == RTC_INVALID_GEOMETRY_ID) {
    throw std::runtime_error("Failed to attach geometry");
  }

  rtcCommitScene(scene_);

  meshes_.push_back(mesh);
  geometry_ids_.push_back(geomety_id);
}
