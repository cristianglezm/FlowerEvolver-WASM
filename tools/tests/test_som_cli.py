import json
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
import flowerevolver as fe
import som


@pytest.fixture(scope="module")
def dataset_file(tmp_path_factory):
    path = tmp_path_factory.mktemp("cli") / "dataset.json"
    assert som.main(["--generate-dataset", "8", "--out", str(path)]) == 0
    return path


def train(dataset_file, out, *extra):
    return som.main(["--train", "--dataset", str(dataset_file), "--out", str(out),
                     "--width", "3", "--height", "2", "--epochs", "6", "--report-every", "3", *extra])


class TestSomCli:
    def test_generate_dataset_writes_n_flowers_and_refuses_to_overwrite(self, dataset_file, capsys):
        assert len(json.loads(dataset_file.read_text())["Generation"]) == 8
        with pytest.raises(SystemExit):
            som.main(["--generate-dataset", "2", "--out", str(dataset_file)])
        assert som.main(["--generate-dataset", "2", "--out", str(dataset_file), "--force"]) == 0
        assert len(json.loads(dataset_file.read_text())["Generation"]) == 2
        som.main(["--generate-dataset", "8", "--out", str(dataset_file), "--force"])

    def test_train_writes_a_loadable_classifier_and_reports_progress(self, dataset_file, tmp_path, capsys):
        out = tmp_path / "fc.json"
        assert train(dataset_file, out) == 0
        c = fe.FlowerClassifier.from_json(out.read_text())
        assert (c.width, c.height) == (3, 2)
        err = capsys.readouterr().err
        assert "training [" in err and "3/6" in err and "6/6" in err and "QE" in err
        with pytest.raises(SystemExit):
            train(dataset_file, out)
        assert train(dataset_file, out, "--force") == 0

    def test_classify_prints_the_cell_as_json(self, dataset_file, tmp_path, capsys):
        model = tmp_path / "fc.json"
        train(dataset_file, model)
        flower = tmp_path / "flower.json"
        flower.write_text(json.dumps({"Flower": json.loads(dataset_file.read_text())["Generation"][0]}))
        capsys.readouterr()
        assert som.main(["--classify", str(flower), "--load-model", str(model)]) == 0
        cell = json.loads(capsys.readouterr().out)
        assert 0 <= cell["x"] < 3 and 0 <= cell["y"] < 2
        assert cell["distance"] >= 0
        flower.write_text("{}")
        assert som.main(["--classify", str(flower), "--load-model", str(model)]) == 1
        assert capsys.readouterr().out.strip() == "null"

    def test_export_writes_a_header_that_embeds_the_classifier(self, dataset_file, tmp_path, capsys):
        model, header = tmp_path / "fc.json", tmp_path / "Resources.hpp"
        train(dataset_file, model)
        assert som.main(["--export", str(model), "--out", str(header)]) == 0
        text = header.read_text()
        assert "fe::resources" in text and "trainedSOM" in text
        embedded = text.split('trainedSOM = "', 1)[1].rsplit('";', 1)[0]
        assert json.loads(embedded.replace('\\"', '"').replace("\\\\", "\\")) == json.loads(model.read_text())
        with pytest.raises(SystemExit):
            som.main(["--export", str(model), "--out", str(header)])
        assert som.main(["--export", str(model), "--out", str(header), "--force"]) == 0
        model.write_text("{}")
        assert som.main(["--export", str(model), "--out", str(header), "--force"]) != 0

    def test_weight_decimals_make_the_written_files_shorter_and_still_load(self, dataset_file, tmp_path, capsys):
        exact, rounded = tmp_path / "exact.json", tmp_path / "rounded.json"
        train(dataset_file, exact)
        assert train(dataset_file, rounded, "--weight-decimals", "4") == 0
        assert len(rounded.read_text()) < len(exact.read_text())
        assert fe.FlowerClassifier.from_json(rounded.read_text()).width == 3
        # the same model, exported both ways
        full, short = tmp_path / "full.hpp", tmp_path / "short.hpp"
        assert som.main(["--export", str(exact), "--out", str(full)]) == 0
        assert som.main(["--export", str(exact), "--out", str(short), "--weight-decimals", "4"]) == 0
        assert len(short.read_text()) * 2 < len(full.read_text())
        embedded = short.read_text().split('trainedSOM = "', 1)[1].rsplit('";', 1)[0]
        loaded = fe.FlowerClassifier.from_json(embedded.replace('\\"', '"').replace("\\\\", "\\"))
        original = fe.FlowerClassifier.from_json(exact.read_text())
        for y in range(2):
            for x in range(3):
                for a, b in zip(original.get_prototype(x, y), loaded.get_prototype(x, y)):
                    assert b == pytest.approx(a, abs=5.1e-5)
        # a value outside 0 to 17 is an error, not a crash
        assert som.main(["--export", str(exact), "--out", str(tmp_path / "bad.hpp"), "--weight-decimals", "99"]) == 2
        assert not (tmp_path / "bad.hpp").exists()

    def test_finetune_changes_the_model_and_reports_before_and_after(self, dataset_file, tmp_path, capsys):
        model, tuned = tmp_path / "fc.json", tmp_path / "fc2.json"
        train(dataset_file, model)
        capsys.readouterr()
        assert som.main(["--finetune", "--load-model", str(model), "--dataset", str(dataset_file),
                         "--out", str(tuned), "--epochs", "4", "--report-every", "2"]) == 0
        err = capsys.readouterr().err
        assert "before:" in err and "after:" in err and "4/4" in err
        assert tuned.read_text() != model.read_text()

    def test_invalid_hyperparameters_exit_with_an_error(self, dataset_file, tmp_path, capsys):
        assert train(dataset_file, tmp_path / "x.json", "--base-lr", "0") == 2
        assert "error:" in capsys.readouterr().err

    def test_a_mode_is_required(self):
        with pytest.raises(SystemExit):
            som.main([])
