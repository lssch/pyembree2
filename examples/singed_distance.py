import numpy as np

import pyembree2

if __name__ == "__main__":
    vertices = np.array(
        [
            [0.0, 0.0, 0.0],
            [1.0, 0.0, 0.0],
            [0.0, 1.0, 0.0],
            [1.0, 1.0, 0.0],
        ],
        dtype=float,
    )

    faces = np.array(
        [
            [0, 1, 2],
            [1, 3, 2],
        ],
        dtype=int,
    )

    mesh = pyembree2.triangle_mesh(vertices=vertices, faces=faces)

    query = np.array(
        [
            [0.5, 0.5, 1.0],
            [2.0, 2.0, 2.0],
        ],
        dtype=float,
    )

    mesh_indices, face_indices, points, singed_distances = pyembree2.distance(
        meshes=[mesh], points=query
    )

    print(f"mesh_indices: {mesh_indices}")
    print(f"face_indices: {face_indices}")
    print(f"points: {points}")
    print(f"singed_distances: {singed_distances}")
