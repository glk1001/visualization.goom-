import cplot
import matplotlib.pyplot as plt
import numpy as np

X_MIN = -2.0
X_MAX = -X_MIN
Y_MIN = X_MIN
Y_MAX = -Y_MIN

VIEWPORT = (0.0, 0.0, 1.0, 1.0)
# VIEWPORT = (0.4, 0.4, 0.6, 0.6)


class Displacement:
    def __init__(self):
        self.width = 0
        self.height = 0
        self.x_min = 0
        self.y_min = 0
        self.x_max = 0
        self.y_max = 0
        self.x_nrm = None
        self.y_nrm = None
        self.u = None
        self.v = None

    def xy_nrm(self, x, y):
        return self.x_nrm[x, y], self.y_nrm[x, y]

    def f(self, x, y):
        return self.u[x, y], self.v[x, y]

    def read_arrays(self):
        with open("/tmp/zoom-tests/zoom-point_00001") as f:
            while True:
                line = f.readline()
                if line.startswith("width:"):
                    w = int(line.split("width:")[1])
                elif line.startswith("height:"):
                    h = int(line.split("height:")[1])
                elif line.startswith("chromaFactor:"):
                    break

            f.readline()

            self.x_min = int(VIEWPORT[0] * float(w))
            self.y_min = int(VIEWPORT[1] * float(h))
            self.x_max = int(VIEWPORT[2] * float(w))
            self.y_max = int(VIEWPORT[3] * float(h))

            self.x_nrm = np.empty((w, h), dtype=np.float32)
            self.y_nrm = np.empty((w, h), dtype=np.float32)
            self.u = np.empty((w, h), dtype=np.float32)
            self.v = np.empty((w, h), dtype=np.float32)

            for y in range(h):
                for x in range(w):
                    line = f.readline()
                    vals = line.split()

                    x_val = int(vals[0])
                    y_val = int(vals[1])
                    x_flt = float(vals[2])
                    y_flt = float(vals[3])
                    u_val = float(vals[4])
                    v_val = float(vals[5])

                    if y_val != y:
                        raise Exception(f"Expected y = {y}, not {y_val}.")
                    if x_val != x:
                        raise Exception(f"Expected x = {x}, not {x_val}.")

                    if (
                        self.y_min < y_val < self.y_max
                        and self.x_min < x_val
                        and x_val < self.x_max
                    ):
                        self.x_nrm[x, y] = x_flt
                        self.y_nrm[x, y] = y_flt
                        self.u[x, y] = u_val
                        self.v[x, y] = v_val

        self.width = w
        self.height = h


def plot_quiver(disp):
    x_scale = disp.width / 4.0
    y_scale = disp.height / 4.0

    x_vals = []
    y_vals = []
    u_vals = []
    v_vals = []
    for x in np.linspace(0, disp.width, 10, endpoint=False):
        x = int(x)

        for y in np.linspace(0, disp.height, 10, endpoint=False):
            y = int(y)

            x_nrm, y_nrm = disp.xy_nrm(x, y)
            x_vals.append(x_nrm)
            y_vals.append(y_nrm)

            x_calc = round(x_scale * (x_nrm + 2.0))
            y_calc = round(y_scale * (y_nrm + 2.0))

            u, v = disp.f(x, y)
            u_vals.append(u - x_nrm)
            v_vals.append(v - y_nrm)

            print(x, y, x_calc, y_calc, x_nrm, y_nrm, u, v)

    x_vals = np.array(x_vals)
    y_vals = np.array(y_vals)
    u_vals = np.array(u_vals)
    v_vals = np.array(v_vals)

    fig = plt.figure(figsize=(9, 9))
    ax = fig.add_subplot()

    # Plotting Vector Field with QUIVER
    ax.quiver(x_vals, y_vals, u_vals, v_vals, color="g")
    plt.title("Complex Field Vectors")

    # plt.xlim(0, width)
    # plt.ylim(0, height)
    plt.xlim(-2.0, +2.0)
    plt.ylim(-2.0, +2.0)

    ax.grid()
    plt.show()


def get_saw_tooth(t: np.ndarray, period: float) -> np.ndarray:
    HALF = 0.5
    t_div_period = t / period
    return HALF + (t_div_period - np.floor(HALF + t_div_period))


def get_triangle(t: np.ndarray, period: float) -> np.ndarray:
    def get_triangle_single(t_val: float, period_val: float) -> float:
        res = t_val % period_val
        if t_val < period_val / 2.0:
            return res
        return period_val - res

    get_triangle_func = np.vectorize(get_triangle_single)
    return get_triangle_func(t, period)


def plot_the_effects(disp: Displacement):
    # def format_coord(x: float, y: float):
    #     return format_complex_coord(combined_effects, x, y)

    x_scale = disp.width / 4.0
    y_scale = disp.height / 4.0

    def f_single(x: float, y: float):
        x_index = round(x_scale * (x + 2.0))
        y_index = round(y_scale * (y + 2.0))
        re_fz, im_fz = disp.f(x_index, y_index)
#        re_fz, im_fz = x, y
        return complex(re_fz, im_fz)

    def f(z_vals: np.ndarray):
        get_fz_func = np.vectorize(f_single)
        return get_fz_func(z_vals.real, z_vals.imag)

    plt.figure(figsize=(9.0, 9.0), dpi=100)
    plt.title("filter plot")

    # plt = cplot.riemann_sphere(f)

    cplt = cplot.plot(
        lambda z: f(z),
        (X_MIN, X_MAX, 99),
        (Y_MIN, Y_MAX, 99),
        # abs_scaling=lambda x: 1.1 * x/(1 + x),
        # abs_scaling=lambda x: get_saw_tooth(x, 2.0),
        abs_scaling=lambda x: get_triangle(x, 2.0),
        contours_abs=2.0,  # None,  # 2.0,
        contours_arg=( -3*np.pi / 4, -np.pi / 2, -np.pi / 4, 0, 0, np.pi / 4, 0, np.pi / 2, 3*np.pi / 4, np.pi),
        emphasize_abs_contour_1=True,
        add_colorbars=True,
        add_axes_labels=True,
        saturation_adjustment=3,
        # min_contour_length = None,
        linewidth=3,
    )

    ax: Axes = cplt.gcf().get_axes()[0]
    #ax.format_coord = format_coord

    plt.show()


displacement = Displacement()
displacement.read_arrays()

#plot_quiver(displacement)
plot_the_effects(displacement)