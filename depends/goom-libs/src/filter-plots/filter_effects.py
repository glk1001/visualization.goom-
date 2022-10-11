from abc import ABC, abstractmethod

import numpy as np


class FilterEffectFunction(ABC):
    def __init__(self, name):
        self.name = name
        self.base_zoom_coeffs = complex(0, 0)

    @abstractmethod
    def f(self, z: np.ndarray, sq_dist_from_zero: np.ndarray):
        pass


class IdentityZoom(FilterEffectFunction):
    def __init__(self):
        super().__init__('Identity Zoom')

    def f(self, z: np.ndarray, sq_dist_from_zero: np.ndarray):
        return self.base_zoom_coeffs


class Amulet(FilterEffectFunction):
    def __init__(self):
        super().__init__('Amulet')
        self.amplitude = 0.9

    def f(self, z: np.ndarray, sq_dist_from_zero: np.ndarray):
        # angle = np.sin(1.0/np.log(abs(np.arctan2(np.imag(z), np.real(z)))))
        return self.base_zoom_coeffs + (self.amplitude * sq_dist_from_zero)


class Wave(FilterEffectFunction):
    def __init__(self, name):
        super().__init__(name)
        self.freq_factor = 2
        self.amplitude = 0.09
        self.periodic_factor = 10
        self.reducer_coeff = 0.001

    def f(self, z: np.ndarray, sq_dist_from_zero: np.ndarray):
        angle = self.freq_factor * sq_dist_from_zero
        reducer = np.exp(-self.reducer_coeff * sq_dist_from_zero)
        return self.base_zoom_coeffs + (
                reducer * self.amplitude * self.periodic_factor * self.f_angle(angle))

    @abstractmethod
    def f_angle(self, angle: np.ndarray):
        return 0


class WaveSine(Wave):
    def __init__(self):
        super().__init__('Wave Sine')

    def f_angle(self, angle: np.ndarray):
        return np.sin(angle)


class WaveTan(Wave):
    def __init__(self):
        super().__init__('Wave Tan')

    def f_angle(self, angle: np.ndarray):
        return np.tan(angle)


class Scrunch(FilterEffectFunction):
    def __init__(self):
        super().__init__('Scrunch')
        self.x_amplitude = 0.1
        self.y_amplitude = 3.0

    def f(self, z: np.ndarray, sq_dist_from_zero: np.ndarray):
        x_zoom_in_coeff = self.base_zoom_coeffs.real + (self.x_amplitude * sq_dist_from_zero)
        y_zoom_in_coeff = self.y_amplitude * x_zoom_in_coeff
        return x_zoom_in_coeff + (y_zoom_in_coeff * 1.0j)


class StrangeSine(FilterEffectFunction):
    def __init__(self):
        super().__init__("Strange Sine")
        self.amplitude = 0.1

    def f(self, z: np.ndarray, sq_dist_from_zero: np.ndarray):
        return self.base_zoom_coeffs + (self.amplitude * np.sin(z ** 3) / z)
