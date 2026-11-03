#!/usr/bin/env python3
"""Runs the study's experiment (experiment/PROTOCOLO.md).

  run_experiment.py run baseline --runs 5     clean code, 5 runs
  run_experiment.py run F01 F02 H01 debug     faults, harmless patches, other build configs
  run_experiment.py run faults harmless       every patch of each folder
  run_experiment.py results                   raw reports -> experiment/results/results.csv

  run_experiment.py mutants list              instrumented build, mutant list -> experiment/results/mutants.csv
  run_experiment.py mutants check             instrumented build with no mutant active: every test must pass
  run_experiment.py mutants run all           every mutant (or: mutants run M0001 M0002 ...)

Each condition is applied alone to a git worktree of the tag tests-frozen (and, for JUCE faults, to a
private clone of JUCE 8.0.9), built, run, recorded under experiment/results/raw/<condition>/run<k>/ and
reverted. Every test runs in its own process, so a crash only affects that test.

Mutants (PROTOCOLO, 7) come from one Mull build (clang 19, experiment/mull.yml). Mull compiles every mutant
into the binaries, and a mutant is switched on by an environment variable named after its Mull id, so no
rebuild is needed: the tests and pluginval run with that one variable set.
"""

import argparse
import csv
import json
import math
import os
import re
import shutil
import subprocess
import sys
import time
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path (__file__).resolve().parent.parent
EXP = ROOT / "experiment"
WORK = EXP / "work"                 # ignored by git
SRC = WORK / "src"                  # worktree of tests-frozen
JUCE = WORK / "JUCE"                # private JUCE clone, patched by the JUCE faults
# TAPSYNTH_EXP_RESULTS points trial runs somewhere else, so they never mix with the study's data
RESULTS_DIR = Path (os.environ.get ("TAPSYNTH_EXP_RESULTS", EXP / "results")).resolve()
RAW = RESULTS_DIR / "raw"
RESULTS = RESULTS_DIR / "results.csv"
TOLERANCES = RESULTS_DIR / "tolerances.json"

MULL_CONFIG = EXP / "mull.yml"
MULL_PLUGIN = ROOT / "tools" / "mull" / "usr" / "lib" / "mull-ir-frontend-19"
MULL_BUILD = WORK / "build-mull"
MUTANTS = RESULTS_DIR / "mutants.csv"
MUTANTS_CHECK = RESULTS_DIR / "mutants-check"   # the instrumented build with no mutant active (not a condition)
# Mull's dispatch calls getenv on every call of a mutated function, so the instrumented tests run much slower.
# The check run has a generous limit; each mutant run then gives every test 10 times its time in the check
# run, at least TEST_TIMEOUT_S (the same rule as Mull's own runner, PROTOCOLO 7)
CHECK_TIMEOUT_S = 30 * 60
MUTANT_TIMEOUT_FACTOR = 10

FROZEN_TAG = "tests-frozen"
TEST_TIMEOUT_S = 60
PLUGINVAL_TIMEOUT_S = 15 * 60

APPROACHES = ["sanidade", "espectral", "temporal", "golden"]
ALL_APPROACHES = APPROACHES + ["pluginval"]

# Build configurations: the reference, plus the two harmless build changes
CONFIGS = {
    "release": { "type": "Release", "cc": None,       "cxx": None },
    "debug":   { "type": "Debug",   "cc": None,       "cxx": None },
    "clang":   { "type": "Release", "cc": "clang-19", "cxx": "clang++-19" },
    "mull":    { "type": "Release", "cc": "clang-19", "cxx": "clang++-19" },   # built by build_mull only
}


def log (msg):
    print (time.strftime ("%H:%M:%S"), msg, flush = True)


def sh (args, cwd = None, check = True, **kw):
    return subprocess.run ([str (a) for a in args], cwd = cwd, check = check, **kw)


def git (repo, *args, check = True, capture = False):
    r = sh (["git", "-C", repo, *args], check = check, capture_output = capture, text = True)
    return r.stdout.strip() if capture else r


# ---------------------------------------------------------------------------------------------- setup

def setup():
    WORK.mkdir (parents = True, exist_ok = True)

    if not SRC.exists():
        git (ROOT, "worktree", "add", "--detach", SRC, FROZEN_TAG)

    if not JUCE.exists():
        git (ROOT, "clone", "-q", ROOT / "tools" / "JUCE", JUCE)

    frozen = git (ROOT, "rev-list", "-n", "1", FROZEN_TAG, capture = True)
    head = git (SRC, "rev-parse", "HEAD", capture = True)

    if head != frozen:
        sys.exit (f"{SRC} is at {head[:7]}, expected {FROZEN_TAG} ({frozen[:7]})")

    juce_version = git (JUCE, "describe", "--tags", capture = True)

    if juce_version != "8.0.9":
        sys.exit (f"{JUCE} is at {juce_version}, expected 8.0.9")


def reset_sources():
    for repo in (SRC, JUCE):
        git (repo, "checkout", "-q", "--", ".")
        git (repo, "clean", "-fdq")


def build_dir (config):
    return WORK / f"build-{config}"


def configure (config):
    bdir = build_dir (config)

    if (bdir / "CMakeCache.txt").exists():
        return

    c = CONFIGS[config]
    args = ["cmake", "-S", SRC, "-B", bdir, "-G", "Ninja", f"-DCMAKE_BUILD_TYPE={c['type']}",
            f"-DTAPSYNTH_JUCE_DIR={JUCE}"]

    if c["cc"]:
        args += [f"-DCMAKE_C_COMPILER={c['cc']}", f"-DCMAKE_CXX_COMPILER={c['cxx']}"]

    sh (args, stdout = subprocess.DEVNULL)


def build (config, log_path):
    configure (config)
    start = time.time()

    with open (log_path, "w") as f:
        r = sh (["cmake", "--build", build_dir (config), "--target", "tapSynth_tests", "tapSynth_VST3"],
                check = False, stdout = f, stderr = subprocess.STDOUT)

    return r.returncode == 0, time.time() - start


def tests_exe (config):
    return build_dir (config) / "tests" / "tapSynth_tests"


def vst3 (config):
    return build_dir (config) / "tapSynth_artefacts" / CONFIGS[config]["type"] / "VST3" / "tapSynth2.vst3"


# ---------------------------------------------------------------------------------------------- conditions

def patch_header (path):
    header = {}

    for line in path.read_text (errors = "replace").splitlines():
        if line.startswith ("diff --git"):
            break

        m = re.match (r"#\s*([\w-]+):\s*(.*)", line)

        if m:
            header[m.group (1)] = m.group (2).strip()

    return header


def resolve (names):
    """Expands condition names into condition dicts."""
    out = []

    for name in names:
        if name == "faults":
            out += resolve (sorted (p.name[:3] for p in (EXP / "faults").glob ("F*.patch")))
        elif name == "harmless":
            out += resolve (sorted (p.name[:3] for p in (EXP / "harmless").glob ("H*.patch")))
        elif name == "baseline":
            out.append ({ "id": "baseline", "kind": "referencia", "config": "release" })
        elif name in ("debug", "clang"):
            out.append ({ "id": name, "kind": "inofensiva", "config": name })
        elif re.fullmatch (r"[FH]\d\d", name):
            folder = "faults" if name[0] == "F" else "harmless"
            matches = list ((EXP / folder).glob (f"{name}-*.patch"))

            if len (matches) != 1:
                sys.exit (f"{name}: expected one patch in experiment/{folder}, found {len (matches)}")

            h = patch_header (matches[0])
            out.append ({ "id": name, "kind": "defeito" if name[0] == "F" else "inofensiva", "config": "release",
                          "patch": matches[0], "origin": h.get ("origin", ""), "category": h.get ("category", "") })
        else:
            sys.exit (f"unknown condition {name}")

    return out


# ---------------------------------------------------------------------------------------------- running

def approach_of (test_name):
    for key, approach in (("Sanity", "sanidade"), ("Spectral", "espectral"), ("Temporal", "temporal"),
                          ("Golden", "golden"), ("AnalysisSelfTest", "selftest")):
        if key in test_name:
            return approach

    raise ValueError (f"no approach for test {test_name}")


def list_tests (exe):
    out = sh ([exe, "--gtest_list_tests"], capture_output = True, text = True, timeout = TEST_TIMEOUT_S).stdout
    names, suite = [], None

    for line in out.splitlines():
        if not line.strip() or line.startswith ("Running main"):
            continue

        if not line.startswith (" "):
            suite = line.split ("#")[0].strip()
        else:
            names.append (suite + line.split ("#")[0].strip())

    return names


def frozen_test_list():
    """Test names of the clean frozen build, cached so that a fault cannot hide tests."""
    cache = WORK / "tests.json"

    if not cache.exists():
        names = list_tests (tests_exe ("release"))
        cache.write_text (json.dumps (names, indent = 1))

    return json.loads (cache.read_text())


def environment (extra):
    return { **os.environ, **extra } if extra else None


def run_test (exe, name, xml_path, env = None, timeout = TEST_TIMEOUT_S):
    start = time.time()

    try:
        r = sh ([exe, f"--gtest_filter={name}", f"--gtest_output=xml:{xml_path}"], check = False,
                capture_output = True, text = True, errors = "replace", timeout = timeout,
                env = environment (env))
    except subprocess.TimeoutExpired:
        return { "name": name, "status": "timeout", "time_s": time.time() - start, "properties": {}, "failures": [] }

    elapsed = time.time() - start
    props, failures, found = {}, [], False

    if xml_path.exists():
        for case in ET.parse (xml_path).iter ("testcase"):
            found = True

            for p in case.iter ("property"):
                props[p.get ("name")] = p.get ("value")

            failures += [f.get ("message", "") for f in case.iter ("failure")]

    if failures:
        status = "fail"
    elif r.returncode == 0 and found:
        status = "pass"
    else:
        status = "crash"

    rec = { "name": name, "status": status, "time_s": elapsed, "returncode": r.returncode,
            "properties": props, "failures": failures }

    if status != "pass":
        rec["output_tail"] = (r.stdout + r.stderr)[-3000:]

    return rec


def run_tests (exe, test_names, out_dir, label, env = None, skip_after_crash = False, timeouts = None):
    """With skip_after_crash, once a test of an approach crashes or times out, the rest of that approach is
    skipped: such a test fails at every tolerance point (PROTOCOLO, 6.3), so the approach's results cannot
    change, and a mutant that hangs would otherwise cost a 60 s timeout per test."""
    log (f"{label}: running {len (test_names)} tests")
    decided = set()

    with open (out_dir / "tests.jsonl", "w") as f:
        for i, name in enumerate (test_names):
            approach = approach_of (name)

            if approach in decided:
                rec = { "name": name, "status": "skipped", "time_s": 0.0, "properties": {}, "failures": [] }
            else:
                timeout = timeouts[name] if timeouts else TEST_TIMEOUT_S
                rec = run_test (exe, name, out_dir / "xml" / f"{i:03d}.xml", env, timeout)

                if timeout != TEST_TIMEOUT_S:
                    rec["timeout_s"] = timeout

                if skip_after_crash and rec["status"] in ("crash", "timeout"):
                    decided.add (approach)

            rec["approach"] = approach
            f.write (json.dumps (rec) + "\n")


def run_pluginval (plugin, out_dir, label, env = None):
    log (f"{label}: running pluginval")
    start = time.time()

    try:
        with open (out_dir / "pluginval.log", "w") as f:
            r = sh ([shutil.which ("bash") or "bash", EXP / "run_pluginval.sh", plugin, out_dir / "pluginval"], check = False,
                    stdout = f, stderr = subprocess.STDOUT, timeout = PLUGINVAL_TIMEOUT_S, env = environment (env))
        status = "pass" if r.returncode == 0 else "fail"
        code = r.returncode
    except subprocess.TimeoutExpired:
        status, code = "timeout", None

    result = { "status": status, "returncode": code, "time_s": time.time() - start }
    (out_dir / "pluginval.json").write_text (json.dumps (result))


def run_condition (cond, run_index, test_names):
    out_dir = RAW / cond["id"] / f"run{run_index}"

    if (out_dir / "meta.json").exists():
        log (f"{cond['id']} run{run_index}: already recorded, skipped")
        return

    if out_dir.exists():
        shutil.rmtree (out_dir)

    (out_dir / "xml").mkdir (parents = True)
    meta = { k: (str (v) if isinstance (v, Path) else v) for k, v in cond.items() }
    meta.update ({ "run": run_index, "started": time.strftime ("%Y-%m-%dT%H:%M:%S"),
                   "frozen_commit": git (SRC, "rev-parse", "HEAD", capture = True) })

    reset_sources()

    try:
        if "patch" in cond:
            target = JUCE if cond["origin"] == "juce" else SRC
            applied = git (target, "apply", cond["patch"], check = False).returncode == 0
        else:
            applied = True

        log (f"{cond['id']} run{run_index}: building ({cond['config']})")
        built, build_s = build (cond["config"], out_dir / "build.log") if applied else (False, 0.0)
        meta.update ({ "applied": applied, "built": built, "build_s": build_s })

        if built:
            label = f"{cond['id']} run{run_index}"
            run_tests (tests_exe (cond["config"]), test_names, out_dir, label)
            run_pluginval (vst3 (cond["config"]), out_dir, label)
    finally:
        reset_sources()

    meta["finished"] = time.strftime ("%Y-%m-%dT%H:%M:%S")
    (out_dir / "meta.json").write_text (json.dumps (meta, indent = 1))
    log (f"{cond['id']} run{run_index}: done")


def cmd_run (args):
    setup()
    conditions = resolve (args.conditions)

    # The clean release build provides the frozen test list and tolerances
    reset_sources()
    ok, _ = build ("release", WORK / "clean-release-build.log")

    if not ok:
        sys.exit ("the clean release build fails, see experiment/work/clean-release-build.log")

    test_names = frozen_test_list()
    save_tolerances()

    for cond in conditions:
        runs = args.runs if cond["id"] == "baseline" else 1

        for k in range (1, runs + 1):
            run_condition (cond, k, test_names)


def cmd_run_prebuilt (args):
    """Runs an already built clean frozen build, e.g. on another OS in CI (PROTOCOLO, 4)."""
    exe, plugin = Path (args.exe).resolve(), Path (args.vst3).resolve()
    test_names = list_tests (exe)

    if not TOLERANCES.exists():
        TOLERANCES.parent.mkdir (parents = True, exist_ok = True)
        out = sh ([exe, "--tapsynth-list-tolerances"], capture_output = True, text = True).stdout
        TOLERANCES.write_text (json.dumps (json.loads (out), indent = 1))

    for k in range (1, args.runs + 1):
        out_dir = RAW / args.id / f"run{k}"

        if (out_dir / "meta.json").exists():
            continue

        (out_dir / "xml").mkdir (parents = True, exist_ok = True)
        meta = { "id": args.id, "kind": "inofensiva", "config": "ci", "platform": sys.platform, "run": k,
                 "frozen_commit": args.frozen_commit, "applied": True, "built": True, "build_s": 0.0,
                 "started": time.strftime ("%Y-%m-%dT%H:%M:%S") }
        label = f"{args.id} run{k}"
        run_tests (exe, test_names, out_dir, label)
        run_pluginval (plugin, out_dir, label)
        meta["finished"] = time.strftime ("%Y-%m-%dT%H:%M:%S")
        (out_dir / "meta.json").write_text (json.dumps (meta, indent = 1))
        log (f"{label}: done")


# ---------------------------------------------------------------------------------------------- mutants

MULL_SYMBOL = re.compile (r"^[0-9a-f]+ [tT] (cxx_[a-z_]+):(/.+):(\d+):(\d+):(\d+):(\d+)$")


def mull_vst3_binary():
    return MULL_BUILD / "tapSynth_artefacts" / "Release" / "VST3" / "tapSynth2.vst3" / "Contents" / "x86_64-linux" / "tapSynth2.so"


def build_mull():
    """One instrumented build of the frozen code: clang 19 with the Mull plugin, Release, scope from mull.yml.

    -ffp-contract=off: by default clang fuses a * b + c into one llvm.fmuladd, which Mull 0.36 does not mutate,
    so most of the arithmetic of the DSP formulas would get no mutants. On x86-64 without FMA (as in the gcc
    reference build) the fused operation is emitted as a multiply and an add anyway, so the output is the same."""
    env = { "MULL_CONFIG": str (MULL_CONFIG) }

    if not (MULL_BUILD / "CMakeCache.txt").exists():
        sh (["cmake", "-S", SRC, "-B", MULL_BUILD, "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Release",
             "-DCMAKE_C_COMPILER=clang-19", "-DCMAKE_CXX_COMPILER=clang++-19",
             f"-DCMAKE_CXX_FLAGS=-g -grecord-command-line -ffp-contract=off -fpass-plugin={MULL_PLUGIN}",
             "-DCMAKE_C_FLAGS=-g -grecord-command-line", f"-DTAPSYNTH_JUCE_DIR={JUCE}"],
            stdout = subprocess.DEVNULL, env = environment (env))

    with open (WORK / "build-mull.log", "w") as f:
        r = sh (["cmake", "--build", MULL_BUILD, "--target", "tapSynth_tests", "tapSynth_VST3"], check = False,
                stdout = f, stderr = subprocess.STDOUT, env = environment (env))

    if r.returncode != 0:
        sys.exit ("the Mull build fails, see experiment/work/build-mull.log")


def mutants_in (binary):
    out = sh (["nm", "--defined-only", binary], capture_output = True, text = True).stdout
    found = {}

    for line in out.splitlines():
        m = MULL_SYMBOL.match (line)

        if m:
            mutator, path, line_no, col = m.group (1), m.group (2), int (m.group (3)), int (m.group (4))
            mull_id = line.split (" ", 2)[2]
            found[mull_id] = (mutator, path, line_no, col)

    return found


def mutant_list():
    """The mutants of the test executable, numbered M0001... in a stable order, written to mutants.csv."""
    in_tests = mutants_in (tests_exe ("mull"))
    in_vst3 = mutants_in (mull_vst3_binary())

    if set (in_tests) != set (in_vst3):
        log (f"warning: {len (set (in_tests) ^ set (in_vst3))} mutants are in only one of the two binaries")

    rows = []

    for mull_id, (mutator, path, line_no, col) in in_tests.items():
        p = Path (path)

        if p.is_relative_to (SRC):
            origin, rel = "projeto", p.relative_to (SRC)
        elif p.is_relative_to (JUCE):
            origin, rel = "juce", p.relative_to (JUCE)
        else:
            sys.exit (f"mutant outside the scope: {mull_id}")

        rows.append ({ "mull_id": mull_id, "mutator": mutator, "origin": origin, "file": str (rel),
                       "line": line_no, "column": col, "in_vst3": int (mull_id in in_vst3) })

    rows.sort (key = lambda r: (r["origin"] != "projeto", r["file"], r["line"], r["column"], r["mutator"]))

    for i, r in enumerate (rows, 1):
        r["id"] = f"M{i:04d}"

    MUTANTS.parent.mkdir (parents = True, exist_ok = True)

    with open (MUTANTS, "w", newline = "") as f:
        w = csv.DictWriter (f, fieldnames = ["id", "origin", "file", "line", "column", "mutator", "in_vst3", "mull_id"])
        w.writeheader()
        w.writerows (rows)

    return rows


def mutant_timeouts():
    """Per-test limits for the mutant runs, from the check run (see MUTANT_TIMEOUT_FACTOR)."""
    path = MUTANTS_CHECK / "tests.jsonl"

    if not path.exists():
        sys.exit ("run 'mutants check' first")

    tests = [json.loads (l) for l in path.read_text().splitlines()]

    if any (t["status"] != "pass" for t in tests):
        sys.exit ("the check run has failing tests: the instrumented build does not match the reference")

    return { t["name"]: max (TEST_TIMEOUT_S, math.ceil (MUTANT_TIMEOUT_FACTOR * t["time_s"])) for t in tests }


def run_mutant (m, test_names, timeouts):
    out_dir = RAW / m["id"] / "run1"

    if (out_dir / "meta.json").exists():
        return

    if out_dir.exists():
        shutil.rmtree (out_dir)

    (out_dir / "xml").mkdir (parents = True)
    env = { m["mull_id"]: "1" }
    meta = { "id": m["id"], "kind": "mutante", "config": "mull", "origin": m["origin"], "category": "",
             "mutator": m["mutator"], "file": m["file"], "line": int (m["line"]), "mull_id": m["mull_id"],
             "run": 1, "applied": True, "built": True, "build_s": 0.0,
             "frozen_commit": git (SRC, "rev-parse", "HEAD", capture = True),
             "started": time.strftime ("%Y-%m-%dT%H:%M:%S") }
    label = f"{m['id']} ({m['mutator']} {m['file']}:{m['line']})"
    run_tests (tests_exe ("mull"), test_names, out_dir, label, env, skip_after_crash = True, timeouts = timeouts)

    if int (m["in_vst3"]):
        run_pluginval (vst3 ("mull"), out_dir, label, env)
    else:
        (out_dir / "pluginval.json").write_text (json.dumps ({ "status": "pass", "returncode": None, "time_s": 0.0,
                                                               "note": "mutant not in the VST3" }))

    meta["finished"] = time.strftime ("%Y-%m-%dT%H:%M:%S")
    (out_dir / "meta.json").write_text (json.dumps (meta, indent = 1))


def cmd_mutants (args):
    setup()
    reset_sources()
    build_mull()

    if args.action == "list":
        rows = mutant_list()
        log (f"{len (rows)} mutants -> {MUTANTS}")

        for key in ("origin", "file", "mutator"):
            counts = {}

            for r in rows:
                counts[r[key]] = counts.get (r[key], 0) + 1

            print (f"\nby {key}:")

            for k, n in sorted (counts.items(), key = lambda kv: -kv[1]):
                print (f"  {n:5d}  {k}")
        return

    test_names = frozen_test_list()

    if args.action == "check":
        if MUTANTS_CHECK.exists():
            shutil.rmtree (MUTANTS_CHECK)

        (MUTANTS_CHECK / "xml").mkdir (parents = True)
        start = time.time()
        run_tests (tests_exe ("mull"), test_names, MUTANTS_CHECK, "mull, no mutant",
                   timeouts = { name: CHECK_TIMEOUT_S for name in test_names })
        run_pluginval (vst3 ("mull"), MUTANTS_CHECK, "mull, no mutant")
        tests = [json.loads (l) for l in (MUTANTS_CHECK / "tests.jsonl").read_text().splitlines()]
        failed = [t["name"] for t in tests if t["status"] != "pass"]
        pv = json.loads ((MUTANTS_CHECK / "pluginval.json").read_text())["status"]
        log (f"{len (tests) - len (failed)}/{len (tests)} tests pass, pluginval {pv}, {time.time() - start:.0f} s")

        for name in failed:
            print ("  failed:", name)
        return

    if not MUTANTS.exists():
        sys.exit ("run 'mutants list' first")

    timeouts = mutant_timeouts()
    rows = list (csv.DictReader (open (MUTANTS)))

    if args.ids != ["all"]:
        wanted = set (args.ids)
        rows = [r for r in rows if r["id"] in wanted]

    for i, m in enumerate (rows, 1):
        log (f"[{i}/{len (rows)}] {m['id']}")
        run_mutant (m, test_names, timeouts)


# ---------------------------------------------------------------------------------------------- results

def save_tolerances():
    if not TOLERANCES.exists():
        TOLERANCES.parent.mkdir (parents = True, exist_ok = True)
        out = sh ([tests_exe ("release"), "--tapsynth-list-tolerances"], capture_output = True, text = True).stdout
        TOLERANCES.write_text (json.dumps (json.loads (out), indent = 1))


def violates (value, limit, bound):
    # nan violates every limit; inf is compared like the test's own assertion does (PROTOCOLO, 6.4)
    if math.isnan (value):
        return True

    return value > limit if bound == "atMost" else value < limit


def is_tolerance_failure (message):
    return " measured " in message and ", limit " in message


def fails_at (test, tolerances, swept = None, swept_value = None):
    """Whether a test fails when tolerance `swept` is set to `swept_value` and the others stay at default."""
    if test["status"] in ("crash", "timeout"):
        return True

    if test["status"] == "pass" and swept is None:
        return False

    values = []

    for key, raw in test["properties"].items():
        name = key.split ("|")[0]

        if name in tolerances:
            values.append ((name, float (raw)))

    def any_violation (sweep_name, sweep_value):
        for name, v in values:
            t = tolerances[name]
            limit = sweep_value if name == sweep_name else t["value"]

            if violates (v, limit, t["bound"]):
                return True

        return False

    if test["status"] == "fail":
        non_tolerance = any (not is_tolerance_failure (m) for m in test["failures"]) or not any_violation (None, None)

        if non_tolerance:
            return True

    return any_violation (swept, swept_value)


def cmd_results (args):
    tolerances = { t["name"]: t for t in json.loads (TOLERANCES.read_text()) }
    swept_by_approach = { "sanidade": "sanity.", "espectral": "spectral.", "temporal": "temporal.", "golden": "golden." }
    rows = []

    for meta_path in sorted (RAW.glob ("*/run*/meta.json")):
        run_dir = meta_path.parent
        meta = json.loads (meta_path.read_text())
        base = { "defect_id": meta["id"], "kind": meta["kind"], "origin": meta.get ("origin", ""),
                 "category": meta.get ("category", ""), "run": meta["run"] }

        if not meta.get ("built"):
            for a in ALL_APPROACHES:
                rows.append ({ **base, "approach": a, "tolerance": "padrao", "detected": "",
                               "status": "nao_compila", "runtime_s": "" })
            continue

        tests = [json.loads (l) for l in (run_dir / "tests.jsonl").read_text().splitlines()]
        selftest_ok = all (t["status"] == "pass" for t in tests if t["approach"] == "selftest")
        status = "ok" if selftest_ok else "invalida"

        for a in APPROACHES:
            mine = [t for t in tests if t["approach"] == a]
            runtime = round (sum (t["time_s"] for t in mine), 3)
            detected = any (fails_at (t, tolerances) for t in mine)
            rows.append ({ **base, "approach": a, "tolerance": "padrao", "detected": int (detected),
                           "status": status, "runtime_s": runtime })

            for name, t in tolerances.items():
                if not name.startswith (swept_by_approach[a]) or len (t["grid"]) < 2:
                    continue

                for g in t["grid"]:
                    d = any (fails_at (x, tolerances, name, g) for x in mine)

                    if g == t["value"] and d != detected:
                        log (f"warning: {meta['id']} run{meta['run']} {name}={g} disagrees with the default result")

                    rows.append ({ **base, "approach": a, "tolerance": f"{name}={g:g}", "detected": int (d),
                                   "status": status, "runtime_s": runtime })

        pv = json.loads ((run_dir / "pluginval.json").read_text())
        rows.append ({ **base, "approach": "pluginval", "tolerance": "padrao", "detected": int (pv["status"] != "pass"),
                       "status": status, "runtime_s": round (pv["time_s"], 3) })

    fields = ["defect_id", "kind", "origin", "category", "approach", "tolerance", "run", "detected", "status", "runtime_s"]

    with open (RESULTS, "w", newline = "") as f:
        w = csv.DictWriter (f, fieldnames = fields)
        w.writeheader()
        w.writerows (rows)

    log (f"{len (rows)} rows -> {RESULTS}")


def main():
    p = argparse.ArgumentParser (description = __doc__, formatter_class = argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers (dest = "cmd", required = True)
    r = sub.add_parser ("run")
    r.add_argument ("conditions", nargs = "+")
    r.add_argument ("--runs", type = int, default = 5, help = "runs of the baseline (default 5)")
    pb = sub.add_parser ("run-prebuilt", help = "run an already built clean frozen build (CI, other OS)")
    pb.add_argument ("id", help = "condition id, e.g. windows or macos")
    pb.add_argument ("--exe", required = True, help = "path to tapSynth_tests")
    pb.add_argument ("--vst3", required = True, help = "path to tapSynth2.vst3")
    pb.add_argument ("--frozen-commit", required = True)
    pb.add_argument ("--runs", type = int, default = 5)
    mu = sub.add_parser ("mutants", help = "Mull mutants: list, check the instrumented build, run")
    mu.add_argument ("action", choices = ["list", "check", "run"])
    mu.add_argument ("ids", nargs = "*", default = ["all"], help = "for run: mutant ids (M0001 ...) or all")
    sub.add_parser ("results")
    args = p.parse_args()
    { "run": cmd_run, "run-prebuilt": cmd_run_prebuilt, "mutants": cmd_mutants, "results": cmd_results }[args.cmd] (args)


if __name__ == "__main__":
    main()
