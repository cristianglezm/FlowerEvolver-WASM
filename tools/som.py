"""Command line front end for the SOM species classifier tools.

    python som.py --generate-dataset 4000 --out dataset.json
    python som.py --train --dataset dataset.json --out fc.json --width 8 --height 6
    python som.py --finetune --load-model fc.json --dataset dataset.json --out fc2.json
    python som.py --classify flower.json --load-model fc.json
    python som.py --export fc.json --out ../include/fe/Resources.hpp --force

A scripted version of the notebook, built on the same som_trainer modules.
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import flowerevolver as fe  # noqa: E402
from som_trainer import cache as feature_cache, dataset, evaluation, parallel  # noqa: E402
from som_trainer.training import train_with_reports  # noqa: E402
from som_trainer.export import resources_hpp_text  # noqa: E402

TRAIN_DEFAULTS = dict(width=5, height=5, epochs=100, base_lr=0.5, base_radius=2.5)
# fine-tuning starts gentler than a fresh training so cells keep their meaning
FINETUNE_DEFAULTS = dict(epochs=20, base_lr=0.3, base_radius=1.0)
AUTO_CACHE = ""  # --cache without a file name


def generate_dataset(num_flowers, seed=42):
    flowers = dataset.generate_synthetic_population(num_flowers, seed=seed)
    return dataset.generation_json_from_flowers(flowers)


def load_dataset(path):
    return dataset.load_generation_json(path)


class Progress:
    """One updating line on a terminal, one plain line per update otherwise."""

    def __init__(self, label, total, stream=None):
        self.label, self.total = label, max(1, total)
        self.stream = stream or sys.stderr
        self.tty = hasattr(self.stream, "isatty") and self.stream.isatty()

    def update(self, done, detail=""):
        filled = int(20 * done / self.total)
        line = f"{self.label} [{'#' * filled}{'-' * (20 - filled)}] {done}/{self.total} {detail}".rstrip()
        if self.tty:
            self.stream.write("\r" + line + ("\n" if done >= self.total else ""))
        else:
            self.stream.write(line + "\n")
        self.stream.flush()


def write_output(path, text, force):
    path = Path(path)
    if path.exists() and not force:
        raise SystemExit(f"{path} already exists, pass --force to overwrite it")
    path.write_text(text)


SAVE_CACHE_EVERY = 500  # new extractions between writes, so an interrupted run keeps its work


def extract_samples(generations_json, environment, cache=None, workers=1):
    """Extracts every flower's features once, up front, skipping unusable ones.

    With a feature cache, a flower that is already in it is not rendered again and
    the cache learns the ones that are not. With workers above 1 (0: one per core) the
    flowers that have to be rendered are rendered in that many processes, see
    som_trainer.parallel: the samples are the same and in the same order.
    """
    flowers = json.loads(generations_json)["Generation"]
    keys = [feature_cache.flower_key(f) for f in flowers] if cache is not None else [None] * len(flowers)
    results = None
    workers = parallel.resolve_workers(workers)
    if workers > 1:
        todo, queued = [], set()
        for flower, key in zip(flowers, keys):
            if cache is not None:
                if key in cache or key in queued:  # equal flowers: the first is rendered, the cache has the rest
                    continue
                queued.add(key)
            todo.append(json.dumps({"Flower": flower}))
        if len(todo) > 1:
            results = parallel.extract_many(todo, environment, workers)
    progress = Progress("extracting", len(flowers))
    samples, skipped, reused, since_save = [], 0, 0, 0
    try:
        for i, flower in enumerate(flowers, 1):
            key = keys[i - 1]
            if cache is not None and key in cache:
                vector = cache.get(key)
                reused += 1
            else:
                if results is not None:
                    vector = next(results)
                else:
                    try:
                        vector = fe.extract_flower_features(json.dumps({"Flower": flower}), environment)
                    except (ValueError, RuntimeError):
                        vector = None
                if cache is not None:
                    cache.put(key, vector)
                    since_save += 1
                    if since_save >= SAVE_CACHE_EVERY:
                        cache.save()
                        since_save = 0
            if vector is None:
                skipped += 1
            else:
                samples.append(vector)
            if i % 25 == 0 or i == len(flowers):
                detail = [f"skipped {skipped}"] if skipped else []
                if cache is not None:
                    detail.append(f"from cache {reused}")
                progress.update(i, ", ".join(detail))
    finally:
        if results is not None:
            results.close()  # an interrupted run does not wait for the flowers that were not started
    if cache is not None:
        cache.save()
    if not samples:
        raise SystemExit("no usable flowers in the dataset")
    return samples


def open_cache(args, environment, dataset_path):
    """The feature cache asked for with --cache, or None. A bare --cache sits next to the dataset."""
    if args.cache is None:
        return None
    path = feature_cache.default_path(dataset_path) if args.cache == AUTO_CACHE else args.cache
    return feature_cache.FeatureCache(path, environment)


def train_with_progress(classifier, samples, epochs, base_lr, base_radius, report_every, label):
    """One decay over `epochs` with a progress line every `report_every` epochs (see som_trainer.training)."""
    progress = Progress(label, epochs)
    return train_with_reports(classifier, samples, epochs, base_lr, base_radius, report_every,
                              lambda done, e: progress.update(done, f"QE {e['mean_quantization_error']:.4f} "
                                                              f"used {e['used_nodes']}/{e['total_nodes']}"))


def print_summary(title, e):
    print(f"{title}: QE {e['mean_quantization_error']:.4f}, "
          f"used nodes {e['used_nodes']}/{e['total_nodes']} ({e['fraction_used']:.1%}), "
          f"busiest node {e['busiest_node_count']}, samples per node {e['samples_per_node_mean']:.2f}",
          file=sys.stderr)


def environment_from(args):
    return fe.Environment(humidity=args.humidity, temperature=args.temperature,
                          altitude=args.altitude, terrain_type=args.terrain_type)


def pick(value, default):
    return default if value is None else value


def non_negative_int(text):
    try:
        value = int(text)
    except ValueError:
        raise argparse.ArgumentTypeError(f"{text!r} is not a number") from None
    if value < 0:
        raise argparse.ArgumentTypeError("must be 0 (one per core) or more")
    return value


def build_parser():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    mode = p.add_mutually_exclusive_group(required=True)
    mode.add_argument("--generate-dataset", type=int, metavar="N", help="write a synthetic {\"Generation\": [...]} file of N flowers")
    mode.add_argument("--train", action="store_true", help="train a new classifier from --dataset")
    mode.add_argument("--finetune", action="store_true", help="train --load-model further on --dataset")
    mode.add_argument("--classify", metavar="FLOWER.json", help="print the species cell of one flower")
    mode.add_argument("--export", metavar="CLASSIFIER.json", help="write --out as the embedded-classifier header (include/fe/Resources.hpp)")
    p.add_argument("--dataset", help="{\"Generation\": [...]} file")
    p.add_argument("--load-model", help="classifier JSON saved by --train/--finetune")
    p.add_argument("--out", help="where to write the dataset/classifier")
    p.add_argument("--force", action="store_true", help="overwrite --out if it exists")
    p.add_argument("--cache", nargs="?", const=AUTO_CACHE, metavar="FILE",
                   help="reuse and update a cache of extracted features; without FILE it is the dataset's name with -cache "
                        "added (dataset.json -> dataset-cache.json). With --generate-dataset the cache is written along with the dataset")
    p.add_argument("--workers", type=non_negative_int, default=1, metavar="N",
                   help="render the flowers in N processes (0: one per core; default 1: no pool). Rendering takes from 50 ms to a "
                        "second per flower and the flowers are independent, so a big dataset is N times faster; the samples are "
                        "the same in the same order. Threads cannot do this: see som_trainer/parallel.py")
    p.add_argument("--seed", type=int, default=42, help="--generate-dataset only; seeds the render parameters, not the genomes")
    p.add_argument("--width", type=int)
    p.add_argument("--height", type=int)
    p.add_argument("--epochs", type=int)
    p.add_argument("--base-lr", type=float, dest="base_lr")
    p.add_argument("--base-radius", type=float, dest="base_radius")
    p.add_argument("--report-every", type=int, dest="report_every", help="epochs per progress update (default: epochs / 20)")
    p.add_argument("--weight-decimals", type=int, dest="weight_decimals", metavar="N",
                   help="round the weights written by --train, --finetune and --export to N decimals (0 to 17) for a much shorter file; "
                        "4 is a good start (absolute rounding: it would flatten weights of the order of 0.0001, so choose it for the scale "
                        "of your features); default: exact")
    p.add_argument("--humidity", type=float, default=0.5)
    p.add_argument("--temperature", type=int, default=20)
    p.add_argument("--altitude", type=int, default=0)
    p.add_argument("--terrain-type", type=int, default=0, dest="terrain_type")
    return p


def need(args, parser, *names):
    for name in names:
        if getattr(args, name) is None:
            parser.error(f"--{name.replace('_', '-')} is required for this mode")


def main(argv=None):
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        if args.workers != 1 and (args.export is not None or args.classify is not None
                                  or (args.generate_dataset is not None and args.cache is None)):
            parser.error("--workers only applies where flowers are rendered: --train, --finetune and "
                         "--generate-dataset with --cache")
        if args.generate_dataset is not None:
            need(args, parser, "out")
            if args.generate_dataset < 1:
                parser.error("--generate-dataset needs at least 1 flower")
            environment = environment_from(args)
            cache = open_cache(args, environment, args.out)
            # both files are checked before either is written
            if cache is not None and cache.path.exists() and not args.force:
                raise SystemExit(f"{cache.path} already exists, pass --force to overwrite it")
            write_output(args.out, generate_dataset(args.generate_dataset, args.seed), args.force)
            print(f"wrote {args.generate_dataset} flowers to {args.out}", file=sys.stderr)
            if cache is not None:
                cache.entries.clear()  # a new dataset: nothing in an old cache can match it
                cache.dirty = True
                extract_samples(Path(args.out).read_text(), environment, cache, args.workers)
                print(f"wrote {len(cache)} feature vectors to {cache.path}", file=sys.stderr)
            return 0

        if args.cache is not None and (args.export is not None or args.classify is not None):
            parser.error("--cache only applies to --generate-dataset, --train and --finetune")

        if args.export is not None:
            need(args, parser, "out")
            # loading validates the file and re-serialises it compactly
            classifier = fe.FlowerClassifier.from_json(Path(args.export).read_text())
            classifier.weight_decimals = args.weight_decimals
            write_output(args.out, resources_hpp_text(classifier.to_json()), args.force)
            print(f"wrote {args.out} ({classifier.width}x{classifier.height} classifier)", file=sys.stderr)
            return 0

        environment = environment_from(args)
        if args.classify is not None:
            need(args, parser, "load_model")
            classifier = fe.FlowerClassifier.from_json(Path(args.load_model).read_text())
            match = classifier.classify(Path(args.classify).read_text(), environment)
            print(json.dumps(None if match is None else {"x": match.x, "y": match.y, "distance": match.distance}))
            return 0 if match is not None else 1

        need(args, parser, "dataset", "out")
        samples = extract_samples(load_dataset(args.dataset), environment, open_cache(args, environment, args.dataset), args.workers)
        if args.train:
            cfg = {k: pick(getattr(args, k), v) for k, v in TRAIN_DEFAULTS.items()}
            fe.SOMTrainingConfig(cfg["width"], cfg["height"], cfg["epochs"], cfg["base_lr"], cfg["base_radius"])  # validates
            classifier = fe.FlowerClassifier(cfg["width"], cfg["height"], environment)
            label, run = "training", cfg
        else:
            need(args, parser, "load_model")
            classifier = fe.FlowerClassifier.from_json(Path(args.load_model).read_text())
            print_summary("before", classifier.evaluate(samples))
            run = {k: pick(getattr(args, k), v) for k, v in FINETUNE_DEFAULTS.items()}
            label = "finetuning"
        every = args.report_every or max(1, run["epochs"] // 20)
        result = train_with_progress(classifier, samples, run["epochs"], run["base_lr"], run["base_radius"], every, label)
        print_summary("after", result)
        classifier.weight_decimals = args.weight_decimals
        write_output(args.out, classifier.to_json(), args.force)
        print(f"wrote {args.out}", file=sys.stderr)
        return 0
    except ValueError as e:
        print(f"error: {e}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
