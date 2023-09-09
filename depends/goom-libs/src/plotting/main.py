import argparse
import os
from typing import Dict, List

import matplotlib.pyplot as plt
import numpy as np

DATA_DIR = "/home/greg/docker/kodi-nexus/.kodi/userdata/addon_data/visualization.goom-pp/goom_dumps"
MIN_TIMELINE_ELEMENTS = 10


def check_for_long_updates(subdirs: List[str], filename: str) -> None:
    print()
    for subdir in subdirs:
        data_array = load_data_from_file(subdir, filename)
        max_val = max(data_array)
        if max_val > 200:
            print(
                f'Large update time in "{os.path.join(subdir, filename)}":  {int(max_val)}'
            )


def get_full_path(subdir: str, filename: str) -> str:
    return os.path.join(DATA_DIR, subdir, filename)


def load_data_from_file(subdir: str, filename: str):
    return np.loadtxt(os.path.join(DATA_DIR, subdir, filename), usecols=1)


def load_data_from_files(subdir_list: List[str], filename: str):
    data_array = load_data_from_file(subdir_list[0], filename)
    for subdir in subdir_list[1:]:
        next_array = load_data_from_file(subdir, filename)
        data_array = np.append(data_array, [next_array])

    return data_array


def get_percent_buckets(data_array) -> Dict[any, int]:
    buckets = dict()

    for data in data_array:
        if data not in buckets:
            buckets[data] = 0
        buckets[data] += 1

    total = len(data_array)
    percent_buckets = dict()
    for bucket in buckets:
        n = buckets[bucket]
        percent_buckets[bucket] = int(round((100.0 * float(n)) / float(total)))

    return percent_buckets


def print_percent_buckets(name: str, buckets) -> None:
    buckets_str = ""
    for bucket in sorted(buckets):
        buckets_str += f"{int(bucket):>2}:{buckets[bucket]:>3}% "

    print(f"{name:>3}: {buckets_str}")


def add_plot(ax, data_array, yaxis_label, ymin, ymax, yticks, line_color) -> None:
    if len(data_array) < MIN_TIMELINE_ELEMENTS:
        raise Exception(
            f"Too few timeline elements: {len(data_array)} (< {MIN_TIMELINE_ELEMENTS})."
        )

    ax.plot(data_array, color=line_color)
    ax.set(ylabel=yaxis_label)
    ax.set_ylim([ymin, ymax])
    ax.set_yticks(yticks)
    ax.spines["right"].set_visible(False)
    ax.spines["top"].set_visible(False)


def get_subdirs_name(subdirs: List[str]) -> str:
    if len(subdirs) == 1:
        name = subdirs[0]
    else:
        name = f"{subdirs[0]}  ...  {subdirs[-1]}"
    return name.replace("_", " ")


class DataType:
    def __init__(
        self, filename: str, label: str, ymin, ymax, yticks, color, print_percent: bool
    ):
        self.filename: str = filename
        self.label: str = label
        self.ymin = ymin
        self.ymax = ymax
        self.yticks = yticks
        self.color = color
        self.print_percent: bool = print_percent

    def get_ymax(self, data_array):
        if self.ymax != -1 or self.ymax > self.ymin:
            return self.ymax
        return max(data_array)

    def get_yticks(self, data_array):
        if self.yticks:
            return self.yticks
        return 0, max(data_array) / 2, max(data_array)


def plot_groups(subdirs: List[str], plot_data: List[List[DataType]], axs) -> None:
    print()

    i = 0
    for data_type in plot_data:
        i = plot_group(subdirs, data_type, axs, i)


def plot_group(subdirs: List[str], group: List[DataType], axs, axs_index: int) -> int:
    for data_type in group:
        data_array = load_data_from_files(subdirs, data_type.filename)

        if data_type.print_percent:
            print_percent_buckets(data_type.label, get_percent_buckets(data_array))

        add_plot(
            axs[axs_index],
            data_array,
            data_type.label,
            data_type.ymin,
            data_type.get_ymax(data_array),
            data_type.get_yticks(data_array),
            data_type.color,
        )

        axs_index += 1

    return axs_index


def plot(subdirs: List[str]) -> None:
    main_data = [
        DataType("goom_states.dat", "st", 0, 43, (0, 20, 43), "red", True),
        DataType("filter_modes.dat", "fil", 0, 18, (0, 10, 18), "green", True),
        DataType("hypercos_overlays.dat", "hyp", 0, 4, (0, 4), "blue", True),
    ]
    sound_data = [
        DataType("goom_volumes.dat", "vol", 0.0, 1.0, (0, 1), "orange", False),
        DataType("goom_powers.dat", "pwr", 0.0, 1.0, (0, 1), "purple", False),
        DataType("times_since_last_goom.dat", "lst", 0, -1, (), "orange", False),
        DataType("times_since_last_big_goom.dat", "big", 0, -1, (), "purple", False),
        DataType("total_gooms_in_current_cycle.dat", "cyc", 0, -1, (), "orange", False),
    ]
    effects_data = [
        DataType("image_velocity_effects.dat", "img", 0, 1, (0, 1), "purple", True),
        DataType("noise_effects.dat", "nse", 0, 1, (0, 1), "orange", True),
        DataType("plane_effects.dat", "pln", 0, 1, (0, 1), "purple", True),
        DataType("rotation_effects.dat", "rot", 0, 1, (0, 1), "orange", True),
        DataType("tan_effects.dat", "tan", 0, 1, (0, 1), "orange", True),
    ]
    buffer_data = [
        DataType("buffer_lerps.dat", "ler", 0.0, 1.0, (0, 1), "red", False),
    ]
    update_times_data = [
        DataType("update_times.dat", "upd", 0, -1, (), "black", False),
    ]

    plot_data = [
        main_data,
        sound_data,
        # effects_data,
        buffer_data,
        # update_times_data,
    ]

    num_subplots = sum(len(data_type) for data_type in plot_data)

    fig = plt.figure(figsize=(10, 8))
    fig.canvas.set_window_title("Goom Timelines")
    gs = fig.add_gridspec(num_subplots, hspace=0.5)
    axs = gs.subplots(sharex=True)
    # Hide x labels and tick labels for all but bottom plot.
    for ax in axs:
        ax.label_outer()

    fig.suptitle(f"Data: {get_subdirs_name(subdirs)}")

    plot_groups(subdirs, plot_data, axs)

    check_for_long_updates(subdirs, "update_times.dat")

    plt.show()


def get_cleaned_subdirs(subdirs: List[str]) -> List[str]:
    return [s.rstrip("/") for s in subdirs]


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=f"Plot Goom timelines in the directory '{DATA_DIR}'."
    )
    parser.add_argument("subdirs", nargs="+")
    args = parser.parse_args()

    plot(get_cleaned_subdirs(args.subdirs))
