import numpy as np

from after_effects import AfterEffectFunction
from filter_effects import FilterEffectFunction


class CombinedEffects:
    def __init__(self, f_filter_effect: FilterEffectFunction, f_after_effect: AfterEffectFunction):
        self.f_filter_effect = f_filter_effect
        self.f_after_effect = f_after_effect

    def get_name(self) -> str:
        return f'{self.f_filter_effect.name} - {self.f_after_effect.name}'

    def f(self, z: np.ndarray):
        fz = self.f_zoom(z)

        return fz
        # return np.clip(fz, -2.0, +2.0)

    def f_zoom(self, z: np.ndarray):
        # print(f'z.shape = {z.shape}')

        absolute_sq_z = np.absolute(z) ** 2

        base_zoom_coeffs = 0.1 + 0.1j
        # base_zoom_coeffs = -1 + -1j
        # base_zoom_coeffs = -0.1 + -0.5j
        # base_zoom_coeffs = 0 + 0j

        self.f_filter_effect.base_zoom_coeffs = base_zoom_coeffs
        self.f_after_effect.base_zoom_coeffs = base_zoom_coeffs

        zoom_in_coeffs = self.f_filter_effect.f(z, absolute_sq_z)
        # return zoom_in_coeffs

        zoom_in_factor = 1.0 + 1.0j - zoom_in_coeffs
        # zoom_in_factor = zoom_in_coeffs

        zoom_in_point = (zoom_in_factor.real * z.real) + (zoom_in_factor.imag * z.imag) * 1.0j
        # return zoom_in_point

        zoom_in_velocity = z - zoom_in_point
        after_effects_velocity = self.f_after_effect.f(z, absolute_sq_z, zoom_in_velocity)
        after_effects_zoom_in_point = zoom_in_point - after_effects_velocity

        return after_effects_zoom_in_point
        # return np.clip(after_effects_zoom_in_point, -2.0, +2.0)
