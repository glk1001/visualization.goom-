import matplotlib.pyplot as plt
import numpy as np


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


displacement = Displacement()
displacement.read_arrays()

x_vals = []
y_vals = []
u_vals = []
v_vals = []
for x in np.linspace(0, displacement.width, 10, endpoint=False):
    x = int(x)
    print(x)

    for y in np.linspace(0, displacement.height, 10, endpoint=False):
        y = int(y)

        x_nrm, y_nrm = displacement.xy_nrm(x, y)
        x_vals.append(x_nrm)
        y_vals.append(y_nrm)

        u, v = displacement.f(x, y)
        u_vals.append(u - x_nrm)
        v_vals.append(v - y_nrm)

        print(x, y, x_nrm, y_nrm, u, v)

x_vals = np.array(x_vals)
y_vals = np.array(y_vals)
u_vals = np.array(u_vals)
v_vals = np.array(v_vals)

fig = plt.figure(figsize=(9, 9))
ax = fig.add_subplot()

# n = 50
# #plt.imshow(array)
# plt.quiver(X[::n,::n],Y[::n,::n],dx[::n,::n], -dy[::n,::n],
#            np.sqrt(dx[::n,::n]**2+dy[::n,::n]**2),
#            units="xy", scale=0.04, cmap="Reds")

# Plotting Vector Field with QUIVER
ax.quiver(x_vals, y_vals, u_vals, v_vals, color="g")
plt.title("Complex Field Vectors")

# plt.xlim(0, width)
# plt.ylim(0, height)
plt.xlim(-2.0, +2.0)
plt.ylim(-2.0, +2.0)

ax.grid()
plt.show()
