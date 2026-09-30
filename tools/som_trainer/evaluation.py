"""
Evaluation metrics for a trained FlowerClassifier:
quantization error, BMU/node utilization, cluster distribution, and (via
plot_prototype_grid in visualize.py) qualitative inspection of what each
node actually learned. Topology/neighborhood behavior is qualitative too --
see the prototype grid: neighboring nodes should look like visually similar
flowers if the SOM organized sensibly, that's not really a single number.
"""
from __future__ import annotations

from collections import Counter

import numpy as np

from .training import train_with_reports


def quantization_errors(classifier, samples: list[list[float]]) -> np.ndarray:
    """Euclidean distance from each sample to its own BMU's prototype
    (matches EvoAI::SelfOrganizingMap's own EuclideanDistance, the default
    distance function FlowerClassifier is built with). One value per
    sample; samples that don't classify (wrong length -- shouldn't happen
    for anything that came out of extract_flower_features()) are dropped,
    not turned into a NaN or a crash."""
    errors = []
    for sample in samples:
        match = classifier.classify_features(sample)
        if match is None:
            continue
        errors.append(match.distance)
    return np.array(errors)


def mean_quantization_error(classifier, samples: list[list[float]]) -> float:
    errors = quantization_errors(classifier, samples)
    return float(errors.mean()) if len(errors) else float("nan")


def node_utilization(classifier, samples: list[list[float]]) -> np.ndarray:
    """width x height grid of how many samples landed on each node (their
    BMU). Rows are y, columns are x, matching how plot_utilization_heatmap()
    displays it directly with imshow."""
    grid = np.zeros((classifier.height, classifier.width), dtype=int)
    for sample in samples:
        match = classifier.classify_features(sample)
        if match is not None:
            grid[match.y, match.x] += 1
    return grid


def utilization_summary(classifier, samples: list[list[float]]) -> dict:
    """A few scalar summaries of node_utilization(), for a quick "is this
    grid size reasonable" read without eyeballing the heatmap: fraction of
    nodes that got at least one sample, and the count for the
    most-crowded node (a grid where one node soaks up most of the
    population is a sign the grid is too big, or P/bias/radius diversity
    in the training set is too low -- everything really does look similar)."""
    grid = node_utilization(classifier, samples)
    total_nodes = grid.size
    used_nodes = int(np.count_nonzero(grid))
    return {
        "total_nodes": total_nodes,
        "used_nodes": used_nodes,
        "fraction_used": used_nodes / total_nodes if total_nodes else float("nan"),
        "busiest_node_count": int(grid.max()) if grid.size else 0,
        "samples_per_node_mean": float(grid.mean()) if grid.size else float("nan"),
    }


def cluster_sizes(classifier, samples: list[list[float]]) -> Counter:
    """How many samples landed on each occupied (x, y) node -- a
    dict-like {(x, y): count}, sorted descending by count when iterated as
    .most_common()."""
    counts: Counter = Counter()
    for sample in samples:
        match = classifier.classify_features(sample)
        if match is not None:
            counts[(match.x, match.y)] += 1
    return counts


def training_curve(
    train_fn,
    samples: list[list[float]],
    epoch_checkpoints: list[int],
    *,
    continue_base_lr: float = 0.3,
    continue_base_radius: float = 1.0,
) -> list[tuple[int, float]]:
    """Mean quantization error at each cumulative epoch count in
    epoch_checkpoints (e.g. [10, 20, 40, 80]), training ONE classifier and
    calling classifier.continue_training() between checkpoints -- a real
    curve for one run, not independent freshly-initialized classifiers per
    point (those would differ in more than epoch count, since each starts
    from different random prototype weights).

    `train_fn(epochs)` must build+return a *fresh* classifier trained for
    exactly `epochs` epochs (e.g. `lambda epochs: train_flower_classifier(
    samples, environment, SOMTrainingConfig(width, height, epochs, base_lr,
    base_radius))`) -- used once, for the first checkpoint. Every later
    checkpoint continues training via continue_training() with
    `continue_base_lr`/`continue_base_radius` for
    (checkpoints[i] - checkpoints[i-1]) more epochs -- these are separate
    from whatever base_lr/base_radius train_fn's own SOMTrainingConfig
    used (that's a closure, not inspectable from here), and don't need to
    match it: continue_training() always restarts its own decay schedule
    from epoch 0 on every call regardless (see its docstring), so there's
    no single "correct" continuation of the first checkpoint's schedule to
    infer anyway. Pick something smaller than the first checkpoint's
    base_lr/base_radius if the intent is "keep refining", which is the
    usual one.

    The curve this gives has a new decay starting at every checkpoint. To see how
    ONE training run with a given number of epochs converges, use
    `training_curve_one_decay()` instead.
    """
    if not epoch_checkpoints:
        return []
    checkpoints = sorted(epoch_checkpoints)

    classifier = train_fn(checkpoints[0])
    curve = [(checkpoints[0], mean_quantization_error(classifier, samples))]

    for prev, cur in zip(checkpoints, checkpoints[1:]):
        classifier.continue_training(
            samples, cur - prev, continue_base_lr, continue_base_radius)
        curve.append((cur, mean_quantization_error(classifier, samples)))

    return curve


def training_curve_one_decay(
    classifier,
    samples: list[list[float]],
    epochs: int,
    base_lr: float,
    base_radius: float,
    report_every: int,
) -> list[tuple[int, float]]:
    """Mean quantization error every `report_every` epochs of ONE training run of `epochs` epochs.

    `classifier` is a fresh one (for example `fe.FlowerClassifier(width, height, environment)`)
    and is trained in place. Unlike `training_curve()`, the learning rate and radius follow a single
    decay from `base_lr` and `base_radius` to the end, so the curve is that of the run you would get
    by training for `epochs` epochs in one call, with no restart at the checkpoints.
    """
    curve: list[tuple[int, float]] = []
    train_with_reports(classifier, samples, epochs, base_lr, base_radius, report_every,
                       lambda done, e: curve.append((done, e["mean_quantization_error"])))
    return curve


def epoch_curve(report) -> list[tuple[int, float]]:
    """The mean distance of the samples to their species at every epoch of a `SOMTrainingReport`.

    One point per epoch, for free: `classifier.continue_training(...)` measures it while it trains. The
    prototypes move during an epoch, so each value differs a little from the quantization error measured
    after that epoch (`training_curve_one_decay()` measures that one, a full pass per checkpoint).
    """
    return [(epoch.epoch + 1, epoch.mean_distance) for epoch in report.epochs]
