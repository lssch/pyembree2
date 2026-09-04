import numpy as np
import trimesh


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
    scene = [sphere, cone, box]
    bounds = trimesh.Scene([sphere, cone, box]).bounds

    n = 200
    points = np.zeros((n, 3))
    points[:, 0] = random_generator.uniform(*bounds[:, 0], n)
    points[:, 1] = random_generator.uniform(*bounds[:, 1], n)
    points[:, 2] = random_generator.uniform(*bounds[:, 2], n)

    if False:
        pcl = trimesh.points.PointCloud(points, colors=[255, 0, 0, 255])
        scene = trimesh.Scene(scene + [pcl])
        scene.show()

    signed_distances = np.zeros((n, len(scene)))
    for i, mesh in enumerate(scene):
        signed_distances[:, i] = trimesh.proximity.signed_distance(mesh, points)

    mesh_indices = np.argmin(np.abs(signed_distances), axis=1)
    signed_distances = signed_distances[np.arange(n), mesh_indices]
