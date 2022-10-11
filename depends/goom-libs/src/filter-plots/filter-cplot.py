import cplot
import matplotlib
import numpy as np
from matplotlib.axes import Axes

import after_effects
import filter_effects
from after_effects import AfterEffectFunction
from filter_effects import FilterEffectFunction


def f_zoom(f_filter_effect: FilterEffectFunction, f_after_effect: AfterEffectFunction,
           z: np.ndarray):
    sq_dist_from_zero = np.absolute(z) ** 2

    base_zoom_coeffs = complex(-0.1, -0.1)
    # base_zoom_coeffs = complex(0, 0)
    f_filter_effect.base_zoom_coeffs = base_zoom_coeffs
    f_after_effect.base_zoom_coeffs = base_zoom_coeffs

    zoom_in_coeffs = f_filter_effect.f(z, sq_dist_from_zero)
    zoom_in_factor = 1 - zoom_in_coeffs
    zoom_in_point = zoom_in_factor * z
    # return zoom_in_point

    zoom_in_velocity = z - zoom_in_point
    after_effects_velocity = f_after_effect.f(z, sq_dist_from_zero, zoom_in_velocity)
    after_effects_zoom_in_point = zoom_in_point - after_effects_velocity

    return after_effects_zoom_in_point
    # return np.clip(after_effects_zoom_in_point, -2.0, +2.0)


class CombinedEffects:
    def __init__(self, f_filter_effect: FilterEffectFunction, f_after_effect: AfterEffectFunction):
        self.f_filter_effect = f_filter_effect
        self.f_after_effect = f_after_effect

    def get_name(self) -> str:
        return f'{self.f_filter_effect.name} - {self.f_after_effect.name}'

    def f(self, z: np.ndarray):
        fz = f_zoom(self.f_filter_effect, self.f_after_effect, z)

        return fz
        # return np.clip(fz, -2.0, +2.0)


def get_combined_effects() -> CombinedEffects:
    identity_zoom_filter_effect = filter_effects.IdentityZoom()
    amulet_filter_effect = filter_effects.Amulet()
    wave_sine_filter_effect = filter_effects.WaveSine()
    wave_tan_filter_effect = filter_effects.WaveTan()
    scrunch_filter_effect = filter_effects.Scrunch()
    strange_sine_filter_effect = filter_effects.StrangeSine()

    zero_after_effect = after_effects.Zero()
    tan_after_effect = after_effects.Tan()

    filter_effect = identity_zoom_filter_effect
    # filter_effect = amulet_filter_effect
    # filter_effect = wave_sine_filter_effect
    # filter_effect = wave_tan_filter_effect
    # filter_effect = scrunch_filter_effect
    # filter_effect = strange_sine_filter_effect

    # after_effect = zero_after_effect
    after_effect = tan_after_effect

    return CombinedEffects(filter_effect, after_effect)


def format_complex_coord(the_combined_effects: CombinedEffects, x: float, y: float):
    fz = the_combined_effects.f(np.array([complex(x, y)]))[0]

    fz_abs = np.absolute(fz)
    fz_real = fz.real
    fz_imag = fz.imag
    if fz_imag >= 0:
        sign = '+'
    else:
        sign = '-'
        fz_imag = -fz_imag

    info = f'f({x:.3f} + {y:.3f}j) = {fz_real:.3f} {sign} {fz_imag:.3f}j  [ |f| = {fz_abs:.3f} ]'
    # print(info)

    return info


def plot_the_effects(the_combined_effects: CombinedEffects):
    def format_coord(x: float, y: float):
        return format_complex_coord(combined_effects, x, y)

    matplotlib.pyplot.figure(figsize=(10.0, 10.0), dpi=100)
    matplotlib.pyplot.title(the_combined_effects.get_name())

    x_min = -2.1
    x_max = -x_min
    y_min = +x_min
    y_max = -y_min

    # plt = cplot.riemann_sphere(f)

    plt = cplot.plot(
            lambda z: the_combined_effects.f(z),
            (x_min, x_max, 1000),
            (y_min, y_max, 1000),
            abs_scaling=lambda x: 1.15 * x / (x + 1),
            contours_abs=2.0,  # None,  # 2.0,
            # contours_arg=(-np.pi / 2, 0, np.pi / 2, np.pi),
            emphasize_abs_contour_1=True,
            add_colorbars=True,
            add_axes_labels=True,
            saturation_adjustment=3,
            # min_contour_length = None,
            linewidth=3,
    )

    ax: Axes = plt.gcf().get_axes()[0]
    ax.format_coord = format_coord

    matplotlib.pyplot.show()


if __name__ == "__main__":
    combined_effects = get_combined_effects()

    plot_the_effects(combined_effects)
