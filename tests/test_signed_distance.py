import numpy as np
import trimesh

import pyembree2


def test_signed_distance_spheres(random_generator):
    sphere = trimesh.creation.icosphere(radius=3.0)
    sphere.apply_translation((4.0, -2.0, 1.0))
    sphere.visual.face_colors = [0, 255, 0, 255]
    cone = trimesh.creation.cone(radius=4.0, height=6.0)
    cone.apply_translation((-3.0, 1.0, 4.0))
    cone.visual.face_colors = [0, 0, 255, 255]
    box = trimesh.creation.box(extents=[5, 3, 7])
    box.apply_translation((2.0, 3.0, -4.0))
    box.visual.face_colors = [255, 255, 0, 255]
    meshes = [sphere, cone, box]
    bounds = trimesh.Scene([sphere, cone, box]).bounds

    n = 200
    points = np.zeros((n, 3))
    points[:, 0] = random_generator.uniform(*bounds[:, 0], n)
    points[:, 1] = random_generator.uniform(*bounds[:, 1], n)
    points[:, 2] = random_generator.uniform(*bounds[:, 2], n)

    if False:
        pcl = trimesh.points.PointCloud(points, colors=[255, 0, 0, 255])
        meshes = trimesh.Scene(meshes + [pcl])
        meshes.show()

    signed_distances = np.zeros((n, len(meshes)))
    for i, mesh in enumerate(meshes):
        signed_distances[:, i] = trimesh.proximity.signed_distance(mesh, points)

    mesh_indices = np.argmin(np.abs(signed_distances), axis=1)
    signed_distances = signed_distances[np.arange(n), mesh_indices]

    (
        pyembree_mesh_indices,
        _,
        _,
        pyembree_singed_distances,
    ) = pyembree2.signed_distance(
        meshes=[
            pyembree2.triangle_mesh(
                vertices=mesh.vertices,
                faces=mesh.faces,
                vertex_normals=mesh.vertex_normals,
            )
            for mesh in meshes
        ],
        points=points,
    )

    np.testing.assert_allclose(-signed_distances, pyembree_singed_distances, atol=1e-6)
    np.testing.assert_equal(mesh_indices, pyembree_mesh_indices)


def test_signed_vs_unsigned_distance_spheres(random_generator):
    sphere = trimesh.creation.icosphere(radius=3.0)
    sphere.apply_translation((4.0, -2.0, 1.0))
    sphere.visual.face_colors = [0, 255, 0, 255]
    cone = trimesh.creation.cone(radius=4.0, height=6.0)
    cone.apply_translation((-3.0, 1.0, 4.0))
    cone.visual.face_colors = [0, 0, 255, 255]
    box = trimesh.creation.box(extents=[5, 3, 7])
    box.apply_translation((2.0, 3.0, -4.0))
    box.visual.face_colors = [255, 255, 0, 255]
    meshes = [sphere, cone, box]
    bounds = trimesh.Scene([sphere, cone, box]).bounds

    n = 200
    points = np.zeros((n, 3))
    points[:, 0] = random_generator.uniform(*bounds[:, 0], n)
    points[:, 1] = random_generator.uniform(*bounds[:, 1], n)
    points[:, 2] = random_generator.uniform(*bounds[:, 2], n)

    (
        signed_mesh_indices,
        signed_face_indices,
        signed_points,
        signed_distances,
    ) = pyembree2.signed_distance(
        meshes=[
            pyembree2.triangle_mesh(
                vertices=mesh.vertices,
                faces=mesh.faces,
                vertex_normals=mesh.vertex_normals,
            )
            for mesh in meshes
        ],
        points=points,
    )

    (
        unsigned_mesh_indices,
        unsigned_face_indices,
        unsigned_points,
        unsigned_distances,
    ) = pyembree2.unsigned_distance(
        meshes=[
            pyembree2.triangle_mesh(
                vertices=mesh.vertices,
                faces=mesh.faces,
                vertex_normals=mesh.vertex_normals,
            )
            for mesh in meshes
        ],
        points=points,
    )

    np.testing.assert_equal(signed_mesh_indices, unsigned_mesh_indices)
    np.testing.assert_equal(signed_face_indices, unsigned_face_indices)
    np.testing.assert_allclose(signed_points, unsigned_points)
    np.testing.assert_allclose(np.abs(signed_distances), unsigned_distances)
