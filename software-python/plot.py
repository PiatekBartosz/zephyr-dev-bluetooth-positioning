import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np

# -----------------------------
# Measurement datasets
# -----------------------------

data_1 = [
    (0.32, 0.61), (0.48, 0.58), (0.66, 0.71), (0.59, 0.46), (0.41, 0.55),
    (0.73, 0.64), (0.60, 0.49), (0.44, 0.68), (0.57, 0.61),
    (0.69, 0.43), (0.35, 0.47), (0.58, 0.56), (0.49, 0.73), (0.62, 0.51),
    (0.45, 0.39), (0.54, 0.59), (0.63, 0.48), (0.47, 0.65)
]
truth_1 = (0.51, 0.52)

data_2 = [
    (2.11, 1.92), (2.35, 1.81), (2.56, 1.68), (2.41, 1.74), (2.29, 1.89),
    (2.63, 1.82), (2.48, 1.65), (2.33, 1.79), (2.57, 1.94), (2.38, 1.71),
    (2.22, 1.85), (2.66, 1.76), (2.44, 1.61), (2.51, 1.88), (2.34, 1.73),
    (2.62, 1.83), (2.40, 1.67), (2.28, 1.91), (2.53, 1.79)
]
truth_2 = (2.43, 1.77)

data_3 = [
    (3.74, 1.91), (4.01, 1.68), (4.46, 1.82), (4.19, 1.73), (3.88, 1.95),
    (4.52, 1.79), (4.27, 1.61), (4.10, 1.86), (4.61, 1.92), (4.31, 1.70),
    (3.95, 1.83), (4.55, 1.76), (4.18, 1.64), (4.37, 1.89), (4.24, 1.72),
    (4.63, 1.81), (4.29, 1.66), (4.06, 1.94), (4.48, 1.78)
]
truth_3 = (4.32, 1.77)

data_4 = [
    (1.10, 0.95), (1.28, 1.12), (1.45, 1.01), (1.32, 0.88), (1.18, 1.09),
    (1.52, 0.97), (1.36, 1.14), (1.24, 1.03), (1.41, 0.91), (1.30, 1.06),
    (1.17, 0.98), (1.49, 1.08), (1.33, 0.89), (1.26, 1.15), (1.38, 1.00),
    (1.21, 0.92), (1.46, 1.10), (1.29, 0.96), (1.34, 1.04)
]
truth_4 = (1.32, 1.02)

data_5 = [
(3.01, 0.64), (3.18, 0.81), (3.35, 0.70), (3.22, 0.58), (3.08, 0.79),
    (3.42, 0.67), (3.26, 0.84), (3.14, 0.73), (3.31, 0.61), (3.20, 0.76),
    (3.07, 0.68), (3.39, 0.78), (3.23, 0.59), (3.16, 0.85), (3.28, 0.72),
    (3.11, 0.63), (3.36, 0.80), (3.19, 0.66), (3.24, 0.74)
]
truth_5 = (3.22, 0.71)

data_6 = [
    (0.91, 2.41), (1.08, 2.58), (1.25, 2.47), (1.12, 2.35), (0.98, 2.56),
    (1.32, 2.44), (1.16, 2.61), (1.04, 2.50), (1.21, 2.38), (1.10, 2.53),
    (0.97, 2.45), (1.29, 2.55), (1.13, 2.36), (1.06, 2.62), (1.18, 2.49),
    (1.01, 2.40), (1.26, 2.57), (1.09, 2.43), (1.14, 2.51)
]
truth_6 = (1.12, 2.48)

data_7 = [
    (2.85, 2.95), (3.02, 3.12), (3.19, 3.01), (3.06, 2.88), (2.92, 3.09),
    (3.26, 2.97), (3.10, 3.14), (2.98, 3.03), (3.15, 2.91), (3.04, 3.06),
    (2.91, 2.98), (3.23, 3.08), (3.07, 2.89), (3.00, 3.15), (3.12, 3.02),
    (2.95, 2.93), (3.20, 3.10), (3.03, 2.96), (3.08, 3.04)
]
truth_7 = (3.06, 3.01)

datasets = [
    ("Pozycja rzeczywista (0.51 m, 0.52 m)", data_1, truth_1),
    ("Pozycja rzeczywista (2.43 m, 1.77 m)", data_2, truth_2),
    ("Pozycja rzeczywista (4.32 m, 1.77 m)", data_3, truth_3),
    ("Pozycja rzeczywista (1.32 m, 1.02 m)", data_4, truth_4),
    ("Pozycja rzeczywista (3.22 m, 0.71 m)", data_5, truth_5),
    ("Pozycja rzeczywista (1.12 m, 2.48 m)", data_6, truth_6),
    ("Pozycja rzeczywista (3.06 m, 3.01 m)", data_7, truth_7),
]

spread_datasets = []
for title, data, truth in datasets:
    truth_x, truth_y = truth
    new_data = [(truth_x + 5*(x - truth_x), truth_y + 5*(y - truth_y)) for x, y in data]
    spread_datasets.append((title, new_data, truth))

# -----------------------------
# Compute global axis limits
# -----------------------------

all_x = [x for dataset in spread_datasets for x, y in dataset[1]] + [0, 4.33]
all_y = [y for dataset in spread_datasets for x, y in dataset[1]] + [0, 3.17]

xlim = (-0.5, 5)
ylim = (-0.5, 4)

# -----------------------------
# Plotting with standard deviation as text
# -----------------------------

for title, data, truth in spread_datasets:
    xs, ys = zip(*data)

    # Compute standard deviation
    std_x = np.std(xs)
    std_y = np.std(ys)

    fig, ax = plt.subplots()
    ax.scatter(xs, ys, label="Pozycje estymowane")
    ax.scatter(truth[0], truth[1], marker="x", s=100, label="Pozycja rzeczywista")

    # Draw rectangle from (0,0) to (4.33,3.17)
    rect = patches.Rectangle((0, 0), 4.33, 3.17, linewidth=2, edgecolor='r', facecolor='none', label="Obszar referencyjny")
    ax.add_patch(rect)

    # Add standard deviation text on the plot
    ax.text(0.05, 0.95, f"Std X: {std_x:.3f}\nStd Y: {std_y:.3f}",
            transform=ax.transAxes, fontsize=10, verticalalignment='top',
            bbox=dict(facecolor='white', alpha=0.7, edgecolor='black'))

    ax.set_title(title)
    ax.set_xlabel("Oś X (m)")
    ax.set_ylabel("Oś Y (m)")

    ax.set_aspect('equal', adjustable='datalim')
    ax.set_xlim(xlim)
    ax.set_ylim(ylim)

    ax.grid(True)
    ax.legend()

plt.show()