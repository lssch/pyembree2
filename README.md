# pyembree2

[![Pipeline Status](https://github.com/lssch/pyembree2/actions/workflows/CICD.yml/badge.svg)](https://github.com/lssch/pyembree2/actions/workflows/CICD.yml)
![Python Version from PEP 621 TOML](https://img.shields.io/python/required-version-toml?tomlFilePath=https%3A%2F%2Fraw.githubusercontent.com%2Flssch%2Fpyembree2%2Frefs%2Fheads%2Fmain%2Fpyproject.toml)
![PyPI Version](https://img.shields.io/pypi/v/pyembree2?pypiBaseUrl=https%3A%2F%2Ftest.pypi.org)
![PyPI Wheel](https://img.shields.io/pypi/wheel/pyembree2?pypiBaseUrl=https%3A%2F%2Ftest.pypi.org)

Yet another incomplete Python wrapper for [Intel Embree](https://www.embree.org/), a high-performance ray tracing library.

> [!NOTE]  
> Note: As all other python wrapper, pyembree2 doen't expose the full Embree API.

## Architecture
pyembree2 is implemented primarily in C++, with Python providing a thin interface to a small set of well-defined functions for **signed-distance queries** and **ray tracing**.

The C++ implementation handles the Embree device, scene, geometry, and query management internally. This means Python users do not need to deal with Embree's low-level device and scene management.

```python
import pyembree2

mesh = pyembree2.triangle_mesh(vertices=..., faces=...)
ret = pyembree2.distance(meshes=[mesh], points=...)
mesh_indices, face_indices, points, singed_distances = ret
```

## Requirements
- [Python](https://www.python.org/) >=3.10
- [NumPy](https://numpy.org/)
- [pytest](https://docs.pytest.org/en/stable/) - for testing
- [Ruff](https://github.com/astral-sh/ruff) - for linting

All dependencies are declared in [`pyproject.toml`](pyproject.toml).

## Installation
### Runtime
For normal use, install pyembree2 directly from [PyPI](https://pypi.org/project/pyembree2/):

```bash
pip install pyembree2
```

### For Developers
This installation is for **contributors** who want to run tests or modify the code.

1. Clone the repository:
    ```bash
    git clone https://github.com/lssch/pyembree2.git
    cd pyembree2
    ```
2. Create a virtual environment (recommended):

    **Linux / macOS**
    ```bash
    python -m venv .venv
    source .venv/bin/activate
    ```

    **Windows**
    ```bash
    python -m venv .venv
    .venv\Scripts\activate
    ```

3. Install the project with development dependencies:
    ```bash
    pip install -e ".[dev]"
    ```
4. Install and run pre-commit hooks:
    ```bash
    pre-commit install
    pre-commit run --all-files
    ```

5. Run the test suite:
    ```bash
    pytest -v tests
    ```

## Examples
### Singed distance
https://github.com/lssch/pyembree2/blob/main/examples/singed_distance.py?plain=1

### Ray tracing
https://github.com/lssch/pyembree2/blob/main/examples/ray_tracing.py?plain=1
