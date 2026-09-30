"""Training with progress reports, shared by `som.py` and the evaluation helpers.

ONE `continue_training()` call trains the whole decay and reports every epoch through
`on_epoch`, so progress can be shown (or a curve recorded) while the result is exactly that
of a single call: no restarts of the decay, no reshuffling per chunk.
"""


def train_with_reports(classifier, samples, epochs, base_lr, base_radius, report_every, on_report=None):
    """Trains `classifier` in place for `epochs` epochs, one decay, in a single call.

    Every `report_every` epochs, and after the last one, `on_report(done, evaluation)` is called with
    the number of epochs done and `classifier.evaluate(samples)` (measured at the end of that epoch).
    Returns the evaluation of the finished classifier.
    """
    if report_every < 1:
        raise ValueError("report_every must be at least 1")

    def on_epoch(epoch):
        done = epoch.epoch + 1
        if on_report is not None and (done % report_every == 0 or done == epochs):
            on_report(done, classifier.evaluate(samples))

    classifier.continue_training(samples, epochs, base_lr, base_radius, total_epochs=epochs, on_epoch=on_epoch)
    return classifier.evaluate(samples)
