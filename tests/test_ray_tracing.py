import numpy as np
import trimesh

import pyembree2


def _generate_rays(num_rays, origin, direction, rng):
    ray_origins = np.tile(origin, (num_rays, 1))
    ray_directions = np.tile(direction, (num_rays, 1))
    ray_directions += rng.uniform(-0.3, 0.3, ray_directions.shape)
    lengths = np.linalg.norm(ray_directions, axis=1)
    ray_directions = ray_directions / lengths[:, None]
    return ray_origins, ray_directions


def test_signed_distance_spheres(random_generator):
    # Create scene
    sphere = trimesh.creation.icosphere(radius=3.0)
    sphere.apply_translation((4.0, -2.0, 1.0))
    sphere.visual.face_colors = [0, 255, 0, 255]
    cone = trimesh.creation.cone(radius=4.0, height=6.0)
    cone.apply_translation((3.0, 1.0, 4.0))
    cone.visual.face_colors = [0, 0, 255, 255]
    box = trimesh.creation.box(extents=[5, 3, 7])
    box.apply_translation((2.0, 3.0, -2.0))
    box.visual.face_colors = [255, 255, 0, 255]
    meshes = [sphere, cone, box]

    # Create rays
    ray_origins0, ray_directions0 = _generate_rays(
        40, (15.0, 15.0, 15.0), (-0.8, -1.0, -0.9), random_generator
    )
    ray_origins1, ray_directions1 = _generate_rays(
        60, (1.0, 20.0, 1.0), (0.0, -1.0, -0.0), random_generator
    )
    ray_origins = np.vstack((ray_origins0, ray_origins1))
    ray_directions = np.vstack((ray_directions0, ray_directions1))
    assert ray_origins.shape[0] == ray_directions.shape[0]
    num_rays = ray_origins.shape[0]

    # Visualization
    if False:
        ray_length = 20.0
        ray_endpoints = ray_origins + (ray_directions * ray_length)
        segments = np.stack([ray_origins, ray_endpoints], axis=1)
        ray_path = trimesh.load_path(segments)
        axes = trimesh.creation.axis(axis_length=5.0)
        scene = trimesh.Scene(meshes + [ray_path, axes])
        scene.show()

    # Run trimesh based raytracing and gather result for each mesh
    masks = np.zeros((num_rays, len(meshes)), dtype=bool)
    face_indices = -1 * np.ones((num_rays, len(meshes)), dtype=int)
    distances = np.full((num_rays, len(meshes)), np.nan)
    us = np.full((num_rays, len(meshes)), np.nan)
    vs = np.full((num_rays, len(meshes)), np.nan)
    for i, mesh in enumerate(meshes):
        locations, index_ray, index_tri = mesh.ray.intersects_location(
            ray_origins=ray_origins,
            ray_directions=ray_directions,
        )

        hit_distances = np.linalg.norm(
            locations - ray_origins[index_ray],
            axis=1,
        )

        triangles = mesh.vertices[mesh.faces[index_tri]]
        barycentrics = trimesh.triangles.points_to_barycentric(
            triangles,
            locations,
        )

        # There can be multiple intersections for the same ray.
        # Keep only the closest one for each ray.
        for ray_idx in np.unique(index_ray):
            hit = np.flatnonzero(index_ray == ray_idx)
            closest = hit[np.argmin(hit_distances[hit])]

            masks[ray_idx, i] = True
            face_indices[ray_idx, i] = index_tri[closest]
            distances[ray_idx, i] = hit_distances[closest]
            us[ray_idx, i] = barycentrics[closest, 1]
            vs[ray_idx, i] = barycentrics[closest, 2]

    # Combine results of all meshes
    expected_mask = np.sum(masks, axis=1) > 0
    min_indices = np.nanargmin(distances[expected_mask], axis=1)
    expected_mesh_indices = -1 * np.ones(num_rays, dtype=int)
    expected_mesh_indices[expected_mask] = min_indices
    expected_face_indices = -1 * np.ones(num_rays, dtype=int)
    expected_face_indices[expected_mask] = face_indices[expected_mask, min_indices]
    expected_distances = np.full(num_rays, np.nan)
    expected_distances[expected_mask] = distances[expected_mask, min_indices]
    expected_us = np.full(num_rays, np.nan)
    expected_us[expected_mask] = us[expected_mask, min_indices]
    expected_vs = np.full(num_rays, np.nan)
    expected_vs[expected_mask] = vs[expected_mask, min_indices]

    mesh_indices, face_indices, distances, us, vs = pyembree2.ray_trace(
        meshes=[
            pyembree2.triangle_mesh(
                vertices=mesh.vertices,
                faces=mesh.faces,
                vertex_normals=mesh.vertex_normals,
            )
            for mesh in meshes
        ],
        origins=ray_origins,
        directions=ray_directions,
    )
    mask = np.isfinite(distances)

    np.testing.assert_equal(mask, expected_mask)
    np.testing.assert_equal(mesh_indices[mask], expected_mesh_indices[expected_mask])
    np.testing.assert_equal(face_indices[mask], expected_face_indices[expected_mask])
    np.testing.assert_allclose(distances, expected_distances, atol=1e-5)
    np.testing.assert_allclose(us, expected_us, atol=1e-5)
    np.testing.assert_allclose(vs, expected_vs, atol=1e-5)
