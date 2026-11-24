import numpy as np
import pytest
import trimesh

import pyembree2


@pytest.fixture(
    params=[
        pytest.param(pyembree2.unsigned_distance, id="unsigned"),
        pytest.param(pyembree2.signed_distance, id="signed"),
    ]
)
def distance_function(request):
    return request.param


def _valid_points(random_generator, n=10):
    return random_generator.uniform(-0.5, 0.5, (n, 3))


def test_signed_distance_requires_vertex_normals(scene, random_generator):
    mesh = trimesh.creation.random_soup(20, seed=random_generator)
    mesh = pyembree2.TriangleMesh(
        vertices=mesh.vertices,
        faces=mesh.faces,
    )
    scene.add_mesh(mesh)

    with pytest.raises(
        RuntimeError,
        match=(r"mesh 0 needs to have vertex_normals for signed_distance calculation"),
    ):
        pyembree2.signed_distance(
            scene=scene,
            points=_valid_points(random_generator),
        )


@pytest.mark.parametrize(
    "points",
    [
        pytest.param(
            np.zeros(3, dtype=np.float32),
            id="wrong-ndim",
        ),
        pytest.param(
            np.zeros((10, 2), dtype=np.float32),
            id="wrong-last-dim",
        ),
    ],
)
def test_points_wrong_ndim(points, distance_function, scene, random_generator):
    mesh = trimesh.creation.random_soup(20, seed=random_generator)
    mesh = pyembree2.TriangleMesh(
        vertices=mesh.vertices, faces=mesh.faces, vertex_normals=mesh.vertex_normals
    )
    scene.add_mesh(mesh)

    with pytest.raises(
        ValueError,
        match=r"points must have shape \(N, 3\)",
    ):
        distance_function(
            scene=scene,
            points=points,
        )
