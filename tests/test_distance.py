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


def _valid_mesh(random_generator, n=20):
    mesh = trimesh.creation.random_soup(n, seed=random_generator)
    return pyembree2.triangle_mesh(
        vertices=mesh.vertices.astype(np.float32),
        faces=mesh.faces.astype(np.intc),
    )


def _valid_points(random_generator, n=10):
    return random_generator.uniform(-0.5, 0.5, (n, 3))


@pytest.mark.parametrize(
    "vertices",
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
def test_vertices_wrong_ndim(vertices, distance_function, random_generator):
    mesh = _valid_mesh(random_generator)
    mesh.vertices = vertices

    with pytest.raises(
        ValueError,
        match=r"mesh 0: vertices must have shape \(N, 3\)",
    ):
        distance_function(
            meshes=[mesh],
            points=_valid_points(random_generator),
        )


def test_vertices_not_contiguous(distance_function, random_generator):
    mesh = _valid_mesh(random_generator)
    mesh.vertices = np.asfortranarray(mesh.vertices)

    with pytest.raises(
        ValueError,
        match=r"mesh 0: vertices must be C-contiguous",
    ):
        distance_function(
            meshes=[mesh],
            points=_valid_points(random_generator),
        )


@pytest.mark.parametrize(
    "faces",
    [
        pytest.param(
            np.zeros(3, dtype=int),
            id="wrong-ndim",
        ),
        pytest.param(
            np.zeros((10, 2), dtype=int),
            id="wrong-last-dim",
        ),
    ],
)
def test_faces_wrong_ndim(faces, distance_function, random_generator):
    mesh = _valid_mesh(random_generator)
    mesh.faces = faces

    with pytest.raises(
        ValueError,
        match=r"mesh 0: faces must have shape \(N, 3\)",
    ):
        distance_function(
            meshes=[mesh],
            points=_valid_points(random_generator),
        )


def test_faces_not_contiguous(distance_function, random_generator):
    mesh = _valid_mesh(random_generator)
    mesh.faces = np.asfortranarray(mesh.faces)

    with pytest.raises(
        ValueError,
        match=r"mesh 0: faces must be C-contiguous",
    ):
        distance_function(
            meshes=[mesh],
            points=_valid_points(random_generator),
        )


def test_signed_distance_requires_vertex_normals(random_generator):
    mesh = _valid_mesh(random_generator)

    with pytest.raises(
        ValueError,
        match=(
            r"mesh 0: vertex_normals must be provided for "
            r"signed_distance calculation"
        ),
    ):
        pyembree2.signed_distance(
            meshes=[mesh],
            points=_valid_points(random_generator),
        )


@pytest.mark.parametrize(
    "normals",
    [
        pytest.param(
            np.zeros(9, dtype=np.float32),
            id="wrong-ndim",
        ),
        pytest.param(
            np.zeros((10, 2), dtype=np.float32),
            id="wrong-last-dim",
        ),
    ],
)
def test_vertex_normals_wrong_ndim(normals, random_generator):
    mesh = _valid_mesh(random_generator)
    mesh.vertex_normals = normals

    with pytest.raises(
        ValueError, match=r"mesh 0: vertex_normals must have shape \(N, 3\)"
    ):
        pyembree2.signed_distance(
            meshes=[mesh],
            points=_valid_points(random_generator),
        )


def test_vertex_normals_wrong_size(random_generator):
    mesh = _valid_mesh(random_generator)
    mesh.vertex_normals = np.zeros(
        (mesh.vertices.shape[0] + 1, 3),
        dtype=np.float32,
    )

    with pytest.raises(
        ValueError,
        match=(
            r"mesh 0: vertex_normals must have the same number "
            r"of rows as vertices"
        ),
    ):
        pyembree2.signed_distance(
            meshes=[mesh],
            points=_valid_points(random_generator),
        )


def test_vertex_normals_not_contiguous(random_generator):
    mesh = _valid_mesh(random_generator)
    mesh.vertex_normals = np.asfortranarray(np.zeros_like(mesh.vertices))

    with pytest.raises(
        ValueError,
        match=r"mesh 0: vertex_normals must be C-contiguous",
    ):
        pyembree2.signed_distance(
            meshes=[mesh],
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
def test_points_wrong_ndim(points, distance_function, random_generator):
    mesh = _valid_mesh(random_generator)
    mesh.vertex_normals = np.zeros(
        (mesh.vertices.shape[0], 3),
        dtype=np.float32,
    )

    with pytest.raises(
        ValueError,
        match=r"points must have shape \(N, 3\)",
    ):
        distance_function(
            meshes=[mesh],
            points=points,
        )
