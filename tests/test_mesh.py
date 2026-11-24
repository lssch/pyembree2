import numpy as np
import pytest
import trimesh

import pyembree2


@pytest.mark.parametrize(
    "vertices",
    [
        pytest.param(
            np.zeros(3, dtype=float),
            id="wrong-ndim",
        ),
        pytest.param(
            np.zeros((10, 2), dtype=float),
            id="wrong-last-dim",
        ),
    ],
)
def test_triangle_mesh_vertices_wrong_ndim(vertices, random_generator):
    mesh = trimesh.creation.random_soup(20, seed=random_generator)

    with pytest.raises(
        ValueError,
        match=r"vertices must have shape \(N, 3\)",
    ):
        mesh = pyembree2.TriangleMesh(
            vertices=vertices, faces=mesh.faces, vertex_normals=mesh.vertex_normals
        )


def test_vertices_not_contiguous(random_generator):
    mesh = trimesh.creation.random_soup(20, seed=random_generator)
    vertices = np.zeros((len(mesh.vertices), 6), dtype=float)[:, ::2]
    assert not vertices.flags.c_contiguous

    with pytest.raises(
        ValueError,
        match=r"vertices must be C-contiguous",
    ):
        mesh = pyembree2.TriangleMesh(
            vertices=vertices, faces=mesh.faces, vertex_normals=mesh.vertex_normals
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
def test_triangle_mesh_faces_wrong_ndim(faces, random_generator):
    mesh = trimesh.creation.random_soup(20, seed=random_generator)

    with pytest.raises(
        ValueError,
        match=r"faces must have shape \(N, 3\)",
    ):
        mesh = pyembree2.TriangleMesh(
            vertices=mesh.vertices, faces=faces, vertex_normals=mesh.vertex_normals
        )


def test_faces_not_contiguous(random_generator):
    mesh = trimesh.creation.random_soup(20, seed=random_generator)
    faces = np.zeros((len(mesh.vertices), 6), dtype=int)[:, ::2]
    assert not faces.flags.c_contiguous

    with pytest.raises(
        ValueError,
        match=r"faces must be C-contiguous",
    ):
        mesh = pyembree2.TriangleMesh(
            vertices=mesh.vertices, faces=faces, vertex_normals=mesh.vertex_normals
        )


@pytest.mark.parametrize(
    "vertex_normals",
    [
        pytest.param(
            np.zeros(3, dtype=float),
            id="wrong-ndim",
        ),
        pytest.param(
            np.zeros((10, 2), dtype=float),
            id="wrong-last-dim",
        ),
    ],
)
def test_triangle_mesh_vertex_normals_ndim(vertex_normals, random_generator):
    mesh = trimesh.creation.random_soup(20, seed=random_generator)

    with pytest.raises(
        ValueError,
        match=r"vertex_normals must have shape \(N, 3\)",
    ):
        mesh = pyembree2.TriangleMesh(
            vertices=mesh.vertices, faces=mesh.faces, vertex_normals=vertex_normals
        )


def test_vertex_normals_not_contiguous(random_generator):
    mesh = trimesh.creation.random_soup(20, seed=random_generator)
    normals = np.zeros((len(mesh.vertex_normals), 6), dtype=float)[:, ::2]
    assert not normals.flags.c_contiguous

    with pytest.raises(
        ValueError,
        match=r"vertex_normals must be C-contiguous",
    ):
        mesh = pyembree2.TriangleMesh(
            vertices=mesh.vertices, faces=mesh.faces, vertex_normals=normals
        )


def test_mesh_has_no_vertex_normals(random_generator):
    mesh = trimesh.creation.random_soup(20, seed=random_generator)
    mesh = pyembree2.TriangleMesh(
        vertices=mesh.vertices,
        faces=mesh.faces,
    )

    assert not mesh.has_vertex_normals


def test_mesh_has_vertex_normals(random_generator):
    mesh = trimesh.creation.random_soup(20, seed=random_generator)
    mesh = pyembree2.TriangleMesh(
        vertices=mesh.vertices, faces=mesh.faces, vertex_normals=mesh.vertex_normals
    )

    assert mesh.has_vertex_normals
