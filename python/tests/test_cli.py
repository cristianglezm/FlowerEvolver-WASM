import json
import shutil
import subprocess
import sys
import pytest

from flowerevolver.cli import build_parser, main


class TestArgParsing:
    """Confirms the exact reference invocations parse without ambiguity -
    -l/-lg/-lf/-ls and -sf/-si/-s3d all share prefixes, which is worth
    covering directly since argparse's abbreviation matching can be
    finicky about exactly this kind of overlap."""

    def test_default_invocation_has_no_lf_or_repr(self):
        args = build_parser().parse_args("-l 3 -r 64 -p 6.0 -b 1.0 -m 5".split())
        assert args.lf is None and args.repr is None
        assert args.l == 3 and args.r == 64 and args.p == pytest.approx(
            6.0) and args.b == pytest.approx(1.0)
        assert args.m == 5

    def test_lf_mode_invocation(self):
        args = build_parser().parse_args(
            "-lf flower.json -l 3 -r 64 -p 6.0 -b 1.0 -m 5 -sf out-flower.json -si out-image.png -s3d out-flower.gltf -se3d out-flower-emissive.gltf".split()
        )
        assert args.lf == "flower.json"
        assert args.sf == "out-flower.json"
        assert args.si == "out-image.png"
        assert args.s3d == "out-flower.gltf"
        assert args.se3d == "out-flower-emissive.gltf"

    def test_repr_mode_invocation(self):
        args = build_parser().parse_args(
            "-repr flower1.json flower2.json -l 3 -r 64 -p 6.0 -b 1.0 -m 2 -sf kidFlower.json -si kidFlower.png".split()
        )
        assert args.repr == ["flower1.json", "flower2.json"]

    def test_defaults_match_documented_values(self):
        args = build_parser().parse_args([])
        assert args.l == 3
        assert args.r == 64
        assert args.p == pytest.approx(6.0)
        assert args.b == pytest.approx(1.0)
        assert args.m == 0


class TestMainEndToEnd:
    """Calls main() directly (in-process, fast) for the bulk of coverage;
    TestSubprocess below covers at least one real subprocess invocation of
    the actual installed console script, to validate that end too."""

    def test_no_args_generates_and_prints_a_flower(self, capsys):
        assert main([]) == 0
        out = capsys.readouterr().out
        parsed = json.loads(out)
        assert len(parsed["Flower"]["dna"]["genomes"]) == 2

    def test_default_mode_with_no_save_flags_prints_json_to_stdout(self, capsys):
        assert main(["-l", "2", "-r", "32", "-p", "6.0", "-b", "1.0"]) == 0
        out = capsys.readouterr().out
        parsed = json.loads(out)
        assert len(parsed["Flower"]["dna"]["genomes"]) == 2

    def test_default_mode_saves_flower_and_image(self, tmp_path):
        flower_path = tmp_path / "flower.json"
        image_path = tmp_path / "flower.png"
        assert main([
            "-l", "2", "-r", "32", "-p", "6.0", "-b", "1.0",
            "-sf", str(flower_path), "-si", str(image_path),
        ]) == 0
        assert flower_path.exists()
        assert image_path.read_bytes()[:8] == b"\x89PNG\r\n\x1a\n"

    def test_s3d_generates_and_saves_a_3d_model(self, tmp_path):
        model_path = tmp_path / "flower.gltf"
        assert main([
            "-l", "2", "-r", "32", "-p", "6.0", "-b", "1.0", "-s3d", str(
                model_path),
        ]) == 0
        parsed = json.loads(model_path.read_text())
        assert parsed["asset"]["version"] == "2.0"

    def test_s3d_is_independent_of_sf_and_si(self, tmp_path):
        model_path = tmp_path / "flower.gltf"
        assert main(["-l", "2", "-r", "32", "-s3d", str(model_path)]) == 0
        assert model_path.exists()
        assert not (tmp_path / "flower.json").exists()
        assert not (tmp_path / "flower.png").exists()

    def test_s3d_with_mutations_saves_only_the_final_model(self, tmp_path):
        seed = tmp_path / "seed.json"
        main(["-l", "2", "-r", "32", "-sf", str(seed)])

        model_path = tmp_path / "flower.gltf"
        assert main([
            "-lf", str(seed), "-l", "2", "-r", "32", "-m", "3", "-s3d", str(model_path),
        ]) == 0
        assert model_path.exists()
        assert sorted(p.name for p in tmp_path.glob(
            "*.gltf")) == ["flower.gltf"]

    def test_se3d_generates_an_emissive_3d_model(self, tmp_path):
        model_path = tmp_path / "flower_emissive.gltf"
        assert main([
            "-l", "2", "-r", "32", "-p", "6.0", "-b", "1.0", "-se3d", str(
                model_path),
        ]) == 0
        parsed = json.loads(model_path.read_text())
        assert parsed["asset"]["version"] == "2.0"
        materials_text = json.dumps(parsed.get("materials", []))
        assert "emissiveFactor" in materials_text
        assert "KHR_materials_emissive_strength" in parsed.get(
            "extensionsUsed", [])

    def test_s3d_and_se3d_together_save_two_distinct_files(self, tmp_path):
        plain_path = tmp_path / "flower.gltf"
        emissive_path = tmp_path / "flower_emissive.gltf"
        assert main([
            "-l", "2", "-r", "32", "-s3d", str(
                plain_path), "-se3d", str(emissive_path),
        ]) == 0
        assert plain_path.exists()
        assert emissive_path.exists()

        plain = json.loads(plain_path.read_text())
        emissive = json.loads(emissive_path.read_text())
        assert "emissiveFactor" not in json.dumps(plain.get("materials", []))
        assert "emissiveFactor" in json.dumps(emissive.get("materials", []))

    def test_lf_mode_loads_and_mutates_saving_only_the_final_result(self, tmp_path):
        seed = tmp_path / "seed.json"
        main(["-l", "2", "-r", "32", "-p", "6.0", "-b", "1.0", "-sf", str(seed)])

        out_json = tmp_path / "flower.json"
        out_img = tmp_path / "flower.png"
        assert main([
            "-lf", str(seed), "-l", "2", "-r", "32", "-p", "6.0", "-b", "1.0",
            "-m", "3", "-sf", str(out_json), "-si", str(out_img),
        ]) == 0
        assert out_json.exists()
        assert out_img.exists()
        assert sorted(p.name for p in tmp_path.glob(
            "*.json")) == ["flower.json", "seed.json"]
        assert sorted(p.name for p in tmp_path.glob("*.png")) == ["flower.png"]

    def test_repr_mode_saves_only_final_mutation(self, tmp_path):
        a, b = tmp_path / "a.json", tmp_path / "b.json"
        main(["-l", "2", "-r", "32", "-sf", str(a)])
        main(["-l", "2", "-r", "32", "-sf", str(b)])

        kid_json = tmp_path / "kidFlower.json"
        kid_png = tmp_path / "kidFlower.png"
        assert main([
            "-repr", str(a), str(b), "-l", "2", "-r", "32",
            "-m", "2", "-sf", str(kid_json), "-si", str(kid_png),
        ]) == 0
        assert kid_json.exists()
        assert kid_png.exists()
        assert sorted(p.name for p in tmp_path.glob("*kidFlower*")
                      ) == ["kidFlower.json", "kidFlower.png"]

    def test_lf_mode_missing_file_returns_clean_error(self, tmp_path, capsys):
        missing = tmp_path / "does-not-exist.json"
        assert main(["-lf", str(missing)]) == 1
        assert "error:" in capsys.readouterr().err

    def test_lf_mode_malformed_json_returns_clean_error(self, tmp_path, capsys):
        bad = tmp_path / "bad.json"
        bad.write_text("not json")
        assert main(["-lf", str(bad)]) == 1
        assert "error:" in capsys.readouterr().err


class TestSubprocess:
    """One real end-to-end invocation of the actual installed console
    script (not just calling main() in-process) - covers the
    [project.scripts] entry point itself, argv handling, and process exit
    codes for real."""

    def test_installed_console_script_runs(self, tmp_path):
        flower_evolver = shutil.which("flower-evolver")
        if flower_evolver is None:
            pytest.skip("flower-evolver console script not on PATH "
                        "(e.g. venv installed but not activated) -- "
                        "test_module_invocation_also_works covers the same "
                        "code path via `python -m` instead.")
        out = tmp_path / "flower.json"
        result = subprocess.run(
            [flower_evolver, "-l", "2", "-r", "32", "-sf", str(out)],
            capture_output=True, text=True,
        )
        assert result.returncode == 0, result.stderr
        assert out.exists()

    def test_module_invocation_also_works(self, tmp_path):
        out = tmp_path / "flower.json"
        result = subprocess.run(
            [sys.executable, "-m", "flowerevolver.cli",
                "-l", "2", "-r", "32", "-sf", str(out)],
            capture_output=True, text=True,
        )
        assert result.returncode == 0, result.stderr
        assert out.exists()
