# Fuzzing

libFuzzer harnesses covering the boundaries where this library first
touches attacker-controlled bytes: genome/flower JSON, and the numeric
environment parameters that go into `fe::Stats`.

## Prerequisites

- **Clang**, not GCC - `-fsanitize=fuzzer` is a Clang/compiler-rt
  feature with no GCC equivalent.
- The usual native deps: `npm run prep:native` (once), same as any other
  native build.

## Build

```bash
npm run build:native:fuzz
```

Runs `bash .devops/build.sh fuzz`, which itself runs:

```bash
cmake -S . -B build/native/fuzz -G Ninja \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DFlowerEvolver_BUILD_FUZZERS=ON \
    -DEvoAI_ROOT="$(pwd)/build/native/EvoAI/build/install" \
    -DJsonBox_ROOT="$(pwd)/build/native/JsonBox/build/install"
cmake --build build/native/fuzz -j 4
```

Produces `build/native/fuzz/bin/fuzz_{flower_from_json,dna_from_json,stats_from_genome}`.
JsonBox/EvoAI don't need to be rebuilt with sanitizers for this to work --
ASan intercepts the process-wide allocator regardless of which
translation unit allocated the memory, so heap bugs anywhere in the call
stack (including inside those two dependencies) still get caught; only
stack/global-buffer bugs specifically *within* their own non-instrumented
code would be missed.

## Run

```bash
build/native/fuzz/bin/fuzz_flower_from_json -max_total_time=60 tests/fuzz/corpus/
```

Each is a standalone libFuzzer binary -- `-help=1` lists every flag; the
ones worth knowing:
- `-max_total_time=N` -- run for N seconds then stop (a smoke test).
  Omit it to run until you kill it (`Ctrl-C`) -- most real bugs worth
  finding take longer than a CI-sized budget.
- `-minimize_crash=1 -exact_artifact_path=<out> <crash-file>` -- shrink
  a crashing input down to the smallest one that still reproduces it.
- Crashing inputs get written to `./crash-<hash>` in the current
  directory -- move genuinely interesting ones into `tests/fuzz/corpus/`
  so future runs seed from them.

## Seed corpus

`tests/fuzz/corpus/` holds a handful of valid genome/DNA/flower JSON
files (`dna_seed.json`, `flower_seed_0.json`, `flower_seed_1.json`,
`flower_seed_2.json`) to give the fuzzer a well-formed starting point
instead of purely random bytes.

## Open issues

None currently tracked against this repo. If a harness finds a crash
that turns out to be a bug in this repo's own code, add a regression
test under `tests/cpp/` alongside the fix rather than tracking it here.
