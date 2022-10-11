import math
from abc import ABC, abstractmethod

import numpy as np

HALF_PI = 0.5 * math.pi


class AfterEffectFunction(ABC):
    def __init__(self, name: str):
        self.name = name
        self.base_zoom_coeffs = complex(0, 0)

    @abstractmethod
    def f(self, z: np.ndarray, sq_dist_from_zero: np.ndarray, velocity: np.ndarray):
        pass


class Zero(AfterEffectFunction):
    def __init__(self):
        super().__init__('No After Effect')

    def f(self, z: np.ndarray, sq_dist_from_zero: np.ndarray, velocity: np.ndarray):
        return 0


class Tan(AfterEffectFunction):
    def __init__(self):
        super().__init__('Tan After Effect')
        self.x_amplitude = 0.5
        self.y_amplitude = 0.5
        self.limiting_factor = 1
        self.limit = self.limiting_factor * HALF_PI

    def f(self, z: np.ndarray, sq_dist_from_zero: np.ndarray, velocity: np.ndarray):
        def get_tan_sq_dist(arg):
            return np.tan(arg)

        tan_arg = np.clip(np.fmod(sq_dist_from_zero, HALF_PI), -self.limit, +self.limit)
        tan_sq_dist = get_tan_sq_dist(tan_arg)
        return self.x_amplitude * (tan_sq_dist * np.real(velocity)) + (
                self.y_amplitude * (tan_sq_dist * np.imag(velocity))) * 1.0j
