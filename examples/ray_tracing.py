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

    origins = np.array(
        [
            [0.5, 0.5, 1.0],
            [2.0, 2.0, 2.0],
        ],
        dtype=float,
    )

    directions = np.array(
        [
            [0.0, 0.0, -1.0],
            [0.0, 0.0, -1.0],
        ],
        dtype=float,
    )

    mesh_indices, face_indices, distances, u, v = pyembree2.ray_trace(
        meshes=[mesh],
        origins=origins,
        directions=directions,
    )

    print(f"mesh_indices: {mesh_indices}")
    print(f"face_indices: {face_indices}")
    print(f"distances: {distances}")
    print(f"u: {u}")
    print(f"v: {v}")
