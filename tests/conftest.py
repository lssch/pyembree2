import numpy as np
import pytest

import pyembree2


@pytest.fixture
def random_generator():
    return np.random.default_rng(seed=42)


@pytest.fixture
def scene():
    return pyembree2.Scene()
