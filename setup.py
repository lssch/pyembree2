import os

import pybind11
from setuptools import Extension, setup


def embree_paths():
    """Locate Embree include/library dirs across platforms."""
    cwd = os.path.abspath(os.path.expanduser(os.path.dirname(__file__)))

    if os.name == "nt":
        # embree search locations on windows
        return (
            [
                "c:/Program Files/Intel/Embree4/include",
                os.path.join(cwd, "embree4", "include"),
            ],
            ["c:/Program Files/Intel/Embree4/lib", os.path.join(cwd, "embree4", "lib")],
        )
    else:
        # embree search locations on posix including brew on macos
        return (
            [
                "/opt/local/include",
                "/opt/homebrew/include",
                os.path.join(cwd, "embree4", "include"),
            ],
            [
                "/opt/local/lib",
                "/opt/homebrew/lib",
                os.path.join(cwd, "embree4", "lib"),
            ],
        )


includes, libraries_dirs = embree_paths()
include_dirs = [pybind11.get_include(), *includes]

ext = Extension(
    "pyembree2",
    sources=["src/pyembree2/singed_distance.cpp", "src/pyembree2/ray_tracing.cpp"],
    include_dirs=include_dirs,
    library_dirs=libraries_dirs,
    libraries=["embree4"],
    language="c++",
    extra_compile_args=["-std=c++17"] if os.name != "nt" else ["/std:c++17"],
)

setup(ext_modules=[ext])
