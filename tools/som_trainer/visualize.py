"""
Matplotlib helpers for inspecting a trained FlowerClassifier. Every
function returns the Figure it drew (doesn't call plt.show() itself) so
notebook cells can just do `plot_x(...)` and let Jupyter's rich display
handle it, or a script can `.savefig(...)` the result instead.
"""
from __future__ import annotations

import matplotlib.pyplot as plt
import numpy as np

SILHOUETTE_SIZE = 16  # FlowerClassifier.silhouette_size, duplicated here to
                       # avoid importing flowerevolver just for one constant.


def _silhouette_of(feature_vector) -> np.ndarray:
    """The silhouette portion of a feature/prototype vector, reshaped back
    into a 16x16 image (row-major, matching how extract_flower_features()
    flattens it)."""
    arr = np.asarray(feature_vector[: SILHOUETTE_SIZE * SILHOUETTE_SIZE])
    return arr.reshape(SILHOUETTE_SIZE, SILHOUETTE_SIZE)


def plot_prototype_grid(classifier, *, cmap: str = "Greens", figsize_per_cell: float = 1.2):
    """One small silhouette image per SOM node, laid out on the actual grid
    -- this is the "qualitative inspection of visually similar flowers"
    check: if training went well, neighboring cells should look like
    visually similar flower shapes, and the grid overall shouldn't be N
    copies of the same blob."""
    w, h = classifier.width, classifier.height
    fig, axes = plt.subplots(h, w, figsize=(w * figsize_per_cell, h * figsize_per_cell), squeeze=False)
    for y in range(h):
        for x in range(w):
            ax = axes[h - 1 - y][x]  # y=0 at the bottom, matching a normal grid/cartesian layout
            silhouette = _silhouette_of(classifier.get_prototype(x, y))
            ax.imshow(silhouette, cmap=cmap, vmin=0.0, vmax=1.0)
            ax.set_xticks([])
            ax.set_yticks([])
            ax.set_title(f"({x},{y})", fontsize=8)
    fig.suptitle(f"Learned prototypes, {w}x{h} grid")
    fig.tight_layout()
    return fig


def plot_utilization_heatmap(utilization_grid: np.ndarray, *, cmap: str = "viridis"):
    """utilization_grid: from evaluation.node_utilization() -- how many
    training samples landed on each node. A heatmap that's mostly one
    bright cell means the grid is bigger than the training set's actual
    diversity supports; a heatmap with a lot of untouched (dark) cells
    means the same thing from the other direction."""
    fig, ax = plt.subplots(figsize=(utilization_grid.shape[1] * 0.8 + 1, utilization_grid.shape[0] * 0.8 + 1))
    im = ax.imshow(utilization_grid, cmap=cmap, origin="lower")
    for y in range(utilization_grid.shape[0]):
        for x in range(utilization_grid.shape[1]):
            count = utilization_grid[y, x]
            ax.text(x, y, str(count), ha="center", va="center",
                    color="white" if count > utilization_grid.max() / 2 else "black")
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.set_title("Node utilization")
    fig.colorbar(im, ax=ax, label="sample count")
    fig.tight_layout()
    return fig


def plot_training_curve(curve: list[tuple[int, float]]):
    """curve: from evaluation.training_curve(), training_curve_one_decay() or epoch_curve()."""
    epochs, errors = zip(*curve)
    fig, ax = plt.subplots(figsize=(6, 4))
    ax.plot(epochs, errors, marker="o" if len(epochs) <= 50 else None)  # a point per epoch is a line, not dots
    ax.set_xlabel("cumulative epochs")
    ax.set_ylabel("mean quantization error")
    ax.set_title("Quantization error vs. training")
    ax.grid(alpha=0.3)
    fig.tight_layout()
    return fig


def plot_grid_size_comparison(results: dict[tuple[int, int], float]):
    """results: {(width, height): mean_quantization_error}, one entry per
    grid size tried -- see evaluation.mean_quantization_error(). Plotted
    against total node count (width*height), since that's the axis that
    actually trades off against "how finely can this population be split
    up" -- a lower quantization error from a bigger grid isn't a free win,
    it can just mean more nodes to memorize the same population with, so
    read this next to plot_utilization_heatmap()/utilization_summary()
    for whichever grid size you're about to pick, not on its own."""
    sizes = sorted(results.keys(), key=lambda wh: wh[0] * wh[1])
    node_counts = [w * h for w, h in sizes]
    errors = [results[s] for s in sizes]
    labels = [f"{w}x{h}" for w, h in sizes]

    fig, ax = plt.subplots(figsize=(6, 4))
    ax.plot(node_counts, errors, marker="o")
    for nc, err, label in zip(node_counts, errors, labels):
        ax.annotate(label, (nc, err), textcoords="offset points", xytext=(4, 4), fontsize=8)
    ax.set_xlabel("grid size (width x height nodes)")
    ax.set_ylabel("mean quantization error")
    ax.set_title("Quantization error vs. grid size")
    ax.grid(alpha=0.3)
    fig.tight_layout()
    return fig
