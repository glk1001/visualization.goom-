import cplot
import matplotlib
import numpy as np
from matplotlib.axes import Axes

import after_effects
import filter_effects
from all_effects import CombinedEffects


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


def get_combined_effects() -> CombinedEffects:
    identity_zoom_filter_effect = filter_effects.IdentityZoom()
    amulet_filter_effect = filter_effects.Amulet()
    wave_sine_filter_effect = filter_effects.WaveSine()
    wave_tan_filter_effect = filter_effects.WaveTan()
    scrunch_filter_effect = filter_effects.Scrunch()
    # strange_sine_filter_effect = filter_effects.StrangeSine()
    power_filter_effect = filter_effects.Power()

    zero_after_effect = after_effects.Zero()
    tan_after_effect = after_effects.Tan()

    # filter_effect = identity_zoom_filter_effect
    # filter_effect = amulet_filter_effect
    # filter_effect = wave_sine_filter_effect
    # filter_effect = wave_tan_filter_effect
    # filter_effect = scrunch_filter_effect
    # filter_effect = strange_sine_filter_effect
    filter_effect = power_filter_effect

    after_effect = zero_after_effect
    # after_effect = tan_after_effect

    return CombinedEffects(filter_effect, after_effect)


if __name__ == "__main__":
    combined_effects = get_combined_effects()

    plot_the_effects(combined_effects)
