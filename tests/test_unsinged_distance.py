import numpy as np
import trimesh

import pyembree2


def test_unsigned_distance_multiple_meshes(random_generator):
    trimesh_0_mesh = trimesh.creation.random_soup(100, seed=random_generator)
    trimesh_1_mesh = trimesh.creation.random_soup(300, seed=random_generator)

    points = random_generator.uniform(-0.5, 0.5, (200, 3))

    trimesh_0_closest_points, trimesh_0_distances, trimesh_0_triangle_id = (
        trimesh_0_mesh.nearest.on_surface(points)
    )
    trimesh_1_closest_points, trimesh_1_distances, trimesh_1_triangle_id = (
        trimesh_1_mesh.nearest.on_surface(points)
    )

    trimesh_mesh_index = np.argmin(
        np.stack([trimesh_0_distances, trimesh_1_distances], axis=0),
        axis=0,
    )

    trimesh_closest_points = np.where(
        trimesh_mesh_index[:, None] == 0,
        trimesh_0_closest_points,
        trimesh_1_closest_points,
    )

    trimesh_distances = np.choose(
        trimesh_mesh_index,
        [trimesh_0_distances, trimesh_1_distances],
    )

    trimesh_triangle_id = np.choose(
        trimesh_mesh_index,
        [trimesh_0_triangle_id, trimesh_1_triangle_id],
    )

    pyembree_0_mesh = pyembree2.triangle_mesh(
        vertices=trimesh_0_mesh.vertices, faces=trimesh_0_mesh.faces
    )
    pyembree_1_mesh = pyembree2.triangle_mesh(
        vertices=trimesh_1_mesh.vertices, faces=trimesh_1_mesh.faces
    )

    (
        pyembree_mesh_index,
        pyembree_triangle_id,
        pyembree_closest_points,
        pyembree_singed_distances,
    ) = pyembree2.unsigned_distance(
        meshes=[pyembree_0_mesh, pyembree_1_mesh], points=points
    )

    np.testing.assert_equal(trimesh_mesh_index, pyembree_mesh_index)
    np.testing.assert_equal(trimesh_triangle_id, pyembree_triangle_id)
    np.testing.assert_allclose(
        trimesh_closest_points, pyembree_closest_points, atol=1e-6
    )
    np.testing.assert_allclose(
        trimesh_distances, np.abs(pyembree_singed_distances), atol=1e-6
    )
