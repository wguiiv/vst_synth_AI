#!/usr/bin/env bash
# Runs pluginval on the built VST3 with the study's fixed settings (decided before the experiment).
# Usage: experiment/run_pluginval.sh [path/to/tapSynth2.vst3] [output dir]
set -euo pipefail

STRICTNESS=10
SEED=42

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PLUGIN="${1:-$ROOT/build/tapSynth_artefacts/Release/VST3/tapSynth2.vst3}"
OUT="${2:-$ROOT/experiment/results/pluginval}"
PLUGINVAL="${PLUGINVAL:-$ROOT/tools/pluginval/pluginval}"

mkdir -p "$OUT"
"$PLUGINVAL" --strictness-level "$STRICTNESS" --random-seed "$SEED" \
    --skip-gui-tests --validate-in-process --output-dir "$OUT" --validate "$PLUGIN"
