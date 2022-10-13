from abc import ABC, abstractmethod

import numpy as np


class FilterEffectFunction(ABC):
    def __init__(self, name):
        self.name = name
        self.base_zoom_coeffs = complex(0, 0)

    @abstractmethod
    def f(self, z: np.ndarray, absolute_sq_z: np.ndarray):
        pass


class IdentityZoom(FilterEffectFunction):
    def __init__(self):
        super().__init__('Identity Zoom')

    def f(self, z: np.ndarray, absolute_sq_z: np.ndarray):
        return self.base_zoom_coeffs


class Amulet(FilterEffectFunction):
    def __init__(self):
        super().__init__('Amulet')
        self.amplitude = 0.9

    def f(self, z: np.ndarray, absolute_sq_z: np.ndarray):
        # angle = np.sin(1.0/np.log(abs(np.arctan2(np.imag(z), np.real(z)))))
        return self.base_zoom_coeffs + (self.amplitude * absolute_sq_z * (1 + 1j))


class Wave(FilterEffectFunction):
    def __init__(self, name):
        super().__init__(name)
        self.freq_factor = 2
        self.amplitude = 0.09
        self.periodic_factor = 10
        self.reducer_coeff = 0.001

    def f(self, z: np.ndarray, absolute_sq_z: np.ndarray):
        reducer = np.exp(-self.reducer_coeff * absolute_sq_z)
        full_factor = reducer * self.amplitude * self.periodic_factor
        angle = self.freq_factor * absolute_sq_z
        return self.base_zoom_coeffs + (full_factor * self.f_angle(angle) * (1 + 1j))

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

    def f(self, z: np.ndarray, absolute_sq_z: np.ndarray):
        x_zoom_in_coeff = self.base_zoom_coeffs.real + (self.x_amplitude * absolute_sq_z)
        y_zoom_in_coeff = self.y_amplitude * x_zoom_in_coeff
        return x_zoom_in_coeff + (y_zoom_in_coeff * 1.0j)


class YOnly(FilterEffectFunction):
    def __init__(self):
        super().__init__('Y Only')
        self.x_freq_factor = 10.1
        self.y_freq_factor = 20.0
        self.x_amplitude = 1.0
        self.y_amplitude = 10.0

    def get_y_only_zoom_in_multiplier(self, z: np.ndarray):
        return np.sin(self.x_freq_factor * z.real) * np.cos(self.y_freq_factor * z.imag)

    def f(self, z: np.ndarray, absolute_sq_z: np.ndarray):
        x_zoom_in_coeff = self.base_zoom_coeffs.real * self.x_amplitude * \
                          self.get_y_only_zoom_in_multiplier(z)

        # return x_zoom_in_coeff + x_zoom_in_coeff * 1.0j

        return x_zoom_in_coeff + (
                    self.base_zoom_coeffs.imag * self.y_amplitude *
                    self.get_y_only_zoom_in_multiplier(z)) * 1.0j


class StrangeSine(FilterEffectFunction):
    def __init__(self):
        super().__init__("Strange Sine")
        self.amplitude = 0.1
        self.factor = 0.01

    def f(self, z: np.ndarray, absolute_sq_z: np.ndarray):
        return self.base_zoom_coeffs + ((self.amplitude * np.sin(z ** 3)) / (self.factor*z))


class Power(FilterEffectFunction):
    def __init__(self):
        super().__init__("Power")
        self.amplitude = 0.001

    def f(self, z: np.ndarray, absolute_sq_z: np.ndarray):
        return self.base_zoom_coeffs + z ** 6 + 1
