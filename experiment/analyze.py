#!/usr/bin/env python3
"""Phase 4 analysis (PROTOCOLO, section 9).

Reads experiment/results/ (results.csv, mutants.csv, tolerances.json and the raw test lists) and writes the
tables of the study to experiment/analysis/: one CSV per table and RESULTADOS.md with all of them. Standard
library only, so the numbers can be regenerated anywhere with:

    python3 experiment/analyze.py

Definitions (PROTOCOLO, sections 3, 6 and 7):
  - an approach detects a condition when it is detected in at least one run;
  - false alarm: an approach detects a harmless condition;
  - possibly equivalent mutant: no approach detects it at the default tolerances;
  - tolerance curve: one tolerance moves through its grid while the others stay at default.
"""

import csv
import json
import math
import re
import statistics
from collections import defaultdict
from itertools import combinations
from pathlib import Path

ROOT = Path (__file__).resolve().parent.parent
EXP = ROOT / "experiment"
RESULTS_DIR = EXP / "results"
RAW = RESULTS_DIR / "raw"
OUT = EXP / "analysis"

APPROACHES = ["sanidade", "espectral", "temporal", "golden", "pluginval"]
TEST_APPROACHES = APPROACHES[:4]
ANY_TEST = "qualquer teste"        # union of the four test strategies
ANY = "qualquer abordagem"         # union of all five
TEST_DIRS = { "sanidade": "sanity", "espectral": "spectral", "temporal": "temporal", "golden": "golden" }
SWEPT_BY_APPROACH = { "spectral.": "espectral", "temporal.": "temporal", "golden.": "golden" }
DEFAULT = "padrao"

MUTATOR_GROUPS = [
    ("aritmético", re.compile (r"^cxx_(add|sub|mul|div|rem|minus)_")),
    ("comparação", re.compile (r"^cxx_(lt|le|gt|ge|eq|ne)_")),
    ("incremento", re.compile (r"^cxx_(pre|post)_(inc|dec)_")),
]


# ---------------------------------------------------------------------------------------------- statistics

def wilson (k, n, z = 1.959964):
    if n == 0:
        return (math.nan, math.nan)

    p = k / n
    d = 1 + z * z / n
    centre = (p + z * z / (2 * n)) / d
    half = z * math.sqrt (p * (1 - p) / n + z * z / (4 * n * n)) / d
    return (max (0.0, centre - half), min (1.0, centre + half))


def upper_gamma_regularized (a, x):
    """Q(a, x) = Γ(a, x) / Γ(a), by series for x < a + 1 and by continued fraction otherwise."""
    if x <= 0:
        return 1.0

    if x < a + 1:
        term = total = 1.0 / a
        ap = a

        for _ in range (10000):
            ap += 1
            term *= x / ap
            total += term

            if abs (term) < abs (total) * 1e-15:
                break

        return 1.0 - total * math.exp (-x + a * math.log (x) - math.lgamma (a))

    tiny = 1e-300
    b = x + 1 - a
    c = 1 / tiny
    d = 1 / b
    h = d

    for i in range (1, 10000):
        an = -i * (i - a)
        b += 2
        d = an * d + b
        d = tiny if abs (d) < tiny else d
        c = b + an / c
        c = tiny if abs (c) < tiny else c
        d = 1 / d
        delta = d * c
        h *= delta

        if abs (delta - 1) < 1e-15:
            break

    return math.exp (-x + a * math.log (x) - math.lgamma (a)) * h


def chi2_sf (x, df):
    return upper_gamma_regularized (df / 2, x / 2)


def cochran_q (matrix):
    """matrix: one row per condition, one 0/1 column per approach. Returns (Q, df, p)."""
    k = len (matrix[0])
    cols = [sum (row[j] for row in matrix) for j in range (k)]
    rows = [sum (row) for row in matrix]
    total = sum (rows)
    denominator = k * total - sum (r * r for r in rows)

    if denominator == 0:
        return (math.nan, k - 1, math.nan)

    q = (k - 1) * (k * sum (c * c for c in cols) - total * total) / denominator
    return (q, k - 1, chi2_sf (q, k - 1))


def mcnemar_exact (a, b):
    """Exact two-sided McNemar test on two paired 0/1 lists. Returns (b, c, p): b = only a detects, c = only b."""
    only_a = sum (1 for x, y in zip (a, b) if x and not y)
    only_b = sum (1 for x, y in zip (a, b) if y and not x)
    n = only_a + only_b

    if n == 0:
        return (only_a, only_b, 1.0)

    tail = sum (math.comb (n, i) for i in range (min (only_a, only_b) + 1)) / 2 ** n
    return (only_a, only_b, min (1.0, 2 * tail))


def holm (pvalues):
    order = sorted (range (len (pvalues)), key = lambda i: pvalues[i])
    adjusted = [0.0] * len (pvalues)
    running = 0.0

    for rank, i in enumerate (order):
        running = max (running, min (1.0, (len (pvalues) - rank) * pvalues[i]))
        adjusted[i] = running

    return adjusted


# ---------------------------------------------------------------------------------------------- data

class Data:
    def __init__ (self):
        self.detected = {}                 # (id, run, approach, tolerance) -> 0/1
        self.runtime = {}                  # (id, run, approach) -> seconds at the default tolerances
        self.conditions = {}               # id -> { kind, origin, category, runs, status }
        self.tolerance_points = defaultdict (list)   # tolerance name -> grid values seen, in grid order

        for r in csv.DictReader (open (RESULTS_DIR / "results.csv")):
            c = self.conditions.setdefault (r["defect_id"], { "kind": r["kind"], "origin": r["origin"],
                                                              "category": r["category"], "runs": set(),
                                                              "status": set() })
            c["runs"].add (int (r["run"]))
            c["status"].add (r["status"])

            if r["status"] != "ok":
                continue

            self.detected[(r["defect_id"], int (r["run"]), r["approach"], r["tolerance"])] = int (r["detected"])

            if r["tolerance"] == DEFAULT:
                self.runtime[(r["defect_id"], int (r["run"]), r["approach"])] = float (r["runtime_s"])

        self.tolerances = { t["name"]: t for t in json.load (open (RESULTS_DIR / "tolerances.json")) }
        self.mutants = { m["id"]: m for m in csv.DictReader (open (RESULTS_DIR / "mutants.csv")) }

    def ids (self, kind, origin = None):
        return sorted (i for i, c in self.conditions.items()
                       if c["kind"] == kind and c["status"] == { "ok" } and (origin is None or c["origin"] == origin))

    def detects (self, cid, approach, tolerance = DEFAULT):
        """Detected in at least one run. ANY_TEST / ANY are the unions at the default tolerances."""
        if approach == ANY_TEST:
            return int (any (self.detects (cid, a) for a in TEST_APPROACHES))

        if approach == ANY:
            return int (any (self.detects (cid, a) for a in APPROACHES))

        runs = self.conditions[cid]["runs"]
        values = [self.detected.get ((cid, r, approach, tolerance)) for r in runs]

        if any (v is None for v in values):
            raise KeyError (f"no result for {cid} {approach} {tolerance}")

        return int (any (values))

    def failing_runs (self, cid, approach):
        return sum (self.detected[(cid, r, approach, DEFAULT)] for r in sorted (self.conditions[cid]["runs"]))


def mutator_group (mutator):
    for name, pattern in MUTATOR_GROUPS:
        if pattern.match (mutator):
            return name

    return "outro"


def count_code_lines (paths):
    """Lines that are neither blank nor only a comment."""
    total = 0

    for path in paths:
        in_block = False

        for line in path.read_text (errors = "replace").splitlines():
            s = line.strip()

            if in_block:
                if "*/" in s:
                    in_block = False
                    s = s.split ("*/", 1)[1].strip()
                else:
                    continue

            if s.startswith ("/*"):
                if "*/" not in s:
                    in_block = True
                continue

            if s and not s.startswith ("//"):
                total += 1

    return total


def source_files (directory):
    return sorted (p for p in directory.rglob ("*") if p.suffix in (".cpp", ".h"))


# ---------------------------------------------------------------------------------------------- formatting

def pct (k, n):
    return "–" if n == 0 else f"{100 * k / n:.1f}%".replace (".", ",")


def ci_text (k, n):
    lo, hi = wilson (k, n)
    return "–" if n == 0 else f"{100 * lo:.1f}–{100 * hi:.1f}%".replace (".", ",")


def p_text (p):
    if math.isnan (p):
        return "–"

    return "< 0,001" if p < 0.001 else f"{p:.3f}".replace (".", ",")


def num (x, digits = 1):
    return f"{x:.{digits}f}".replace (".", ",")


def md_table (header, rows):
    lines = ["| " + " | ".join (header) + " |", "| " + " | ".join ("---" for _ in header) + " |"]
    lines += ["| " + " | ".join (str (c) for c in row) + " |" for row in rows]
    return "\n".join (lines)


# ---------------------------------------------------------------------------------------------- tolerance chart

# Reference palette of the dataviz method (light mode only: the figure is for print). Series slots 1 and 2,
# validated together (all pairs): CVD ΔE 24.7, normal-vision ΔE 33.6, contrast 4.30 and 3.12 on the surface.
SURFACE, INK, INK_2, MUTED = "#fcfcfb", "#0b0b0b", "#52514e", "#898781"
GRID, AXIS, BAND = "#e1e0d9", "#c3c2b7", "#f0efec"
SERIES = [("mutantes", "#2a78d6"), ("defeitos", "#eb6834")]
FONT = "system-ui, -apple-system, 'Segoe UI', sans-serif"
SUPERSCRIPT = str.maketrans ("-0123456789", "⁻⁰¹²³⁴⁵⁶⁷⁸⁹")
PANEL_TITLES = {
    "spectral.pitch": "Altura",          "spectral.stopband": "Atenuação na banda de corte",
    "spectral.passband": "Banda de passagem", "spectral.aliasing": "Aliasing",
    "temporal.time": "Tempo do envelope",   "temporal.release": "Tempo de release",
    "temporal.sustain": "Nível de sustain",  "golden.maxAbsDiff": "Diferença máxima (golden)",
}


def tick_label (value, unit):
    """Short grid labels so they fit side by side: seconds in ms, the golden grid as powers of ten."""
    if unit == "s":
        return f"{value * 1000:g}"

    if unit == "linear" and 0 < value < 0.1:
        return "10" + f"{round (math.log10 (value))}".translate (SUPERSCRIPT)

    return f"{value:g}".replace (".", ",")


def unit_label (unit):
    return { "s": "ms", "linear": "amplitude", "cents": "cents", "dB": "dB" }[unit]


def esc (text):
    return str (text).replace ("&", "&amp;").replace ("<", "&lt;").replace (">", "&gt;")


def tolerance_chart (curve_rows, n_faults, n_mutants, n_harmless):
    """Small multiples, one panel per swept tolerance: detection (%) of mutants and faults against the grid,
    strictest value on the left. Grid points where harmless conditions fail sit on a gray band labelled with
    how many of them fail. One y axis per panel, all on the same 0-100% scale."""
    panels = defaultdict (list)

    for name, unit, bound, approach, value, is_default, faults_k, mutants_k, _, alarms, _, ref_fails in curve_rows:
        panels[name].append ({ "unit": unit, "approach": approach, "value": value, "default": is_default,
                               "faults": faults_k, "mutants": mutants_k, "alarms": alarms, "ref": ref_fails })

    cols, panel_w, panel_h = 4, 250, 200
    left, right, top, bottom = 44, 14, 50, 46          # plot inset inside a panel
    header_h, legend_h = 80, 34
    width = cols * panel_w
    height = header_h + legend_h + math.ceil (len (panels) / cols) * panel_h + 8
    plot_w, plot_h = panel_w - left - right, panel_h - top - bottom
    out = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}" '
           f'role="img" aria-labelledby="t d" font-family="{FONT}">',
           '<title id="t">Detecção por tolerância, uma tolerância por vez</title>',
           f'<desc id="d">Taxa de detecção de {n_mutants} mutantes e {n_faults} defeitos em cada ponto da grade de '
           'cada tolerância, com as demais no padrão. Faixas cinza marcam os pontos em que condições inofensivas '
           'falham. Os valores estão na seção 8 de RESULTADOS.md e em curva_tolerancia.csv.</desc>',
           f'<rect width="{width}" height="{height}" fill="{SURFACE}"/>',
           f'<text x="16" y="28" font-size="16" font-weight="600" fill="{INK}">Detecção por tolerância, '
           'uma tolerância por vez</text>',
           f'<text x="16" y="50" font-size="12" fill="{INK_2}">Cada painel varia uma tolerância, da mais rigorosa '
           '(esquerda) para a mais frouxa, com as demais no padrão.</text>',
           f'<text x="16" y="66" font-size="12" fill="{INK_2}">Sanidade e pluginval não têm grade de tolerância e '
           'ficam de fora.</text>']

    # Legend: always present for two series, one row above the panels
    lx, ly = 16, header_h + 12

    for label, color in SERIES:
        n = n_mutants if label == "mutantes" else n_faults
        out += [f'<line x1="{lx}" y1="{ly}" x2="{lx + 18}" y2="{ly}" stroke="{color}" stroke-width="2" stroke-linecap="round"/>',
                f'<circle cx="{lx + 9}" cy="{ly}" r="4" fill="{color}" stroke="{SURFACE}" stroke-width="2"/>',
                f'<text x="{lx + 24}" y="{ly + 4}" font-size="12" fill="{INK_2}">{label} (n = {n})</text>']
        lx += 160

    out += [f'<rect x="{lx}" y="{ly - 8}" width="16" height="16" rx="2" fill="{BAND}"/>',
            f'<text x="{lx + 22}" y="{ly + 4}" font-size="12" fill="{INK_2}">ponto com alarmes falsos (número: '
            f'condições inofensivas que falham, de {n_harmless})</text>']

    for index, (name, points) in enumerate (panels.items()):
        ox = (index % cols) * panel_w
        oy = header_h + legend_h + (index // cols) * panel_h
        px, py = ox + left, oy + top
        step = plot_w / len (points)
        x_of = lambda i: px + step * (i + 0.5)
        y_of = lambda rate: py + plot_h * (1 - rate)
        unit = points[0]["unit"]
        out.append (f'<text x="{ox + 12}" y="{oy + 18}" font-size="12" font-weight="600" fill="{INK}">'
                    f'{esc (PANEL_TITLES.get (name, name))}</text>')
        out.append (f'<text x="{ox + 12}" y="{oy + 32}" font-size="11" fill="{MUTED}">{esc (name)} · '
                    f'{points[0]["approach"]}</text>')

        for i, p in enumerate (points):
            if p["alarms"]:
                out.append (f'<rect x="{px + step * i + 1}" y="{py}" width="{step - 2}" height="{plot_h}" fill="{BAND}">'
                            f'<title>{p["alarms"]} de {n_harmless} condições inofensivas falham'
                            f'{"; o código limpo também falha" if p["ref"] else ""}</title></rect>')
                # Count at the foot of the band: the lines sit high (strict points detect everything)
                out.append (f'<text x="{x_of (i)}" y="{py + plot_h - 6}" font-size="11" text-anchor="middle" fill="{INK_2}">'
                            f'{p["alarms"]}</text>')

        for frac in (0, 0.5, 1):
            y = y_of (frac)
            out.append (f'<line x1="{px}" y1="{y}" x2="{px + plot_w}" y2="{y}" stroke="{AXIS if frac == 0 else GRID}" '
                        'stroke-width="1"/>')
            out.append (f'<text x="{px - 6}" y="{y + 4}" font-size="11" text-anchor="end" fill="{MUTED}" '
                        f'style="font-variant-numeric: tabular-nums">{int (frac * 100)}%</text>')

        for i, p in enumerate (points):
            weight = ' font-weight="600"' if p["default"] else ""
            ink = INK if p["default"] else MUTED
            out.append (f'<text x="{x_of (i)}" y="{py + plot_h + 16}" font-size="11" text-anchor="middle" '
                        f'fill="{ink}"{weight}>{esc (tick_label (p["value"], unit))}</text>')

            if p["default"]:
                out.append (f'<text x="{x_of (i)}" y="{py + plot_h + 29}" font-size="10" text-anchor="middle" '
                            f'fill="{INK_2}">padrão</text>')

        out.append (f'<text x="{px + plot_w}" y="{py + plot_h + 42}" font-size="10" text-anchor="end" '
                    f'fill="{MUTED}">{unit_label (unit)}</text>')

        for label, color in SERIES:
            key, n = ("mutants", n_mutants) if label == "mutantes" else ("faults", n_faults)
            path = " ".join (f'{"M" if i == 0 else "L"}{x_of (i):.1f},{y_of (p[key] / n):.1f}' for i, p in enumerate (points))
            out.append (f'<path d="{path}" fill="none" stroke="{color}" stroke-width="2" stroke-linejoin="round" '
                        'stroke-linecap="round"/>')

            for i, p in enumerate (points):
                tip = (f'{name} = {tick_label (p["value"], unit)} {unit_label (unit)}{" (padrão)" if p["default"] else ""}: '
                       f'{label} {p[key]}/{n} ({pct (p[key], n)})')
                out.append (f'<circle cx="{x_of (i):.1f}" cy="{y_of (p[key] / n):.1f}" r="4" fill="{color}" '
                            f'stroke="{SURFACE}" stroke-width="2"><title>{esc (tip)}</title></circle>')
                # Larger invisible hit target for the tooltip
                out.append (f'<circle cx="{x_of (i):.1f}" cy="{y_of (p[key] / n):.1f}" r="12" fill="transparent">'
                            f'<title>{esc (tip)}</title></circle>')

            if index == 0:
                # Direct labels in the first panel, above the start of each line (text in ink, not the series color)
                # mutants above their line, faults below theirs, so neither label touches the other line's markers
                dy = -9 if key == "mutants" else 18
                out.append (f'<text x="{px + 4}" y="{y_of (points[0][key] / n) + dy:.1f}" font-size="11" '
                            f'fill="{INK_2}">{label}</text>')

    out.append ("</svg>")
    (OUT / "curva_tolerancia.svg").write_text ("\n".join (out) + "\n")


def write_csv (name, header, rows):
    with open (OUT / name, "w", newline = "") as f:
        w = csv.writer (f)
        w.writerow (header)
        w.writerows (rows)


# ---------------------------------------------------------------------------------------------- analysis

def main():
    OUT.mkdir (exist_ok = True)
    d = Data()
    report = ["# Resultados do experimento",
              "",
              "Gerado por `experiment/analyze.py` a partir de `experiment/results/`. Não edite à mão: rode o script de novo.",
              "Taxas com intervalo de confiança de 95% de Wilson. Uma abordagem detecta uma condição quando detecta em pelo menos uma execução."]

    faults = d.ids ("defeito")
    mutants = d.ids ("mutante")
    equivalent = [m for m in mutants if not d.detects (m, ANY)]
    mutants_ne = [m for m in mutants if m not in equivalent]
    harmless = d.ids ("inofensiva")
    reference = d.ids ("referencia")
    sets = [("defeitos", faults), ("mutantes", mutants), ("mutantes sem os possivelmente equivalentes", mutants_ne)]
    columns = APPROACHES + [ANY_TEST, ANY]

    # ---- 1. validity
    by_status = defaultdict (lambda: defaultdict (int))

    for c in d.conditions.values():
        by_status[c["kind"]]["/".join (sorted (c["status"]))] += 1

    reference_failures = [(cid, r, a) for cid in reference for r in sorted (d.conditions[cid]["runs"])
                          for a in APPROACHES if d.detected[(cid, r, a, DEFAULT)]]
    report += ["", "## 1. Validade das execuções", "",
               md_table (["Tipo", "Condições", "Situação"],
                         [[k, sum (v.values()), ", ".join (f"{s}: {n}" for s, n in v.items())]
                          for k, v in sorted (by_status.items())]),
               "",
               f"Referência (código limpo no Linux): {sum (len (d.conditions[c]['runs']) for c in reference)} execuções, "
               f"{len (reference_failures)} falhas nas tolerâncias padrão."]

    # ---- 2. detection per approach and origin (QP1, QP2, QP4)
    det_rows = []
    md_rows = []

    for set_name, ids in sets:
        for origin in (None, "projeto", "juce"):
            sub = [i for i in ids if origin is None or d.conditions[i]["origin"] == origin]
            label = origin or "todos"

            for a in columns:
                k = sum (d.detects (i, a) for i in sub)
                lo, hi = wilson (k, len (sub))
                det_rows.append ([set_name, label, a, k, len (sub), round (k / len (sub), 4) if sub else "",
                                  round (lo, 4), round (hi, 4)])

            md_rows.append ([set_name, label, len (sub)] +
                            [f"{sum (d.detects (i, a) for i in sub)} ({pct (sum (d.detects (i, a) for i in sub), len (sub))})"
                             for a in columns])

    write_csv ("deteccao.csv", ["conjunto", "origem", "abordagem", "detectados", "n", "taxa", "ic95_inf", "ic95_sup"],
               det_rows)

    ci_rows = [[set_name, a, f"{sum (d.detects (i, a) for i in ids)}/{len (ids)}",
                pct (sum (d.detects (i, a) for i in ids), len (ids)), ci_text (sum (d.detects (i, a) for i in ids), len (ids))]
               for set_name, ids in sets for a in columns]
    report += ["", "## 2. Detecção por abordagem e por origem (QP1, QP2, QP4)", "",
               "Detectados (taxa) em cada conjunto. \"Possivelmente equivalentes\" são os mutantes que nenhuma abordagem "
               f"detecta nas tolerâncias padrão ({len (equivalent)} de {len (mutants)}; seção 7 do protocolo).", "",
               md_table (["Conjunto", "Origem", "n"] + columns, md_rows),
               "", "Com intervalo de confiança:", "",
               md_table (["Conjunto", "Abordagem", "Detectados", "Taxa", "IC 95%"], ci_rows)]

    # ---- 3. condition x approach matrix
    fault_matrix = [[i, d.conditions[i]["origin"], d.conditions[i]["category"]] + [d.detects (i, a) for a in columns]
                    for i in faults]
    write_csv ("matriz_defeitos.csv", ["id", "origem", "categoria"] + columns, fault_matrix)

    mutant_matrix = []

    for i in mutants:
        m = d.mutants[i]
        mutant_matrix.append ([i, m["origin"], m["file"], m["line"], m["mutator"], mutator_group (m["mutator"])] +
                              [d.detects (i, a) for a in columns] + [int (i in equivalent)])

    write_csv ("matriz_mutantes.csv", ["id", "origem", "arquivo", "linha", "operador", "grupo"] + columns +
               ["possivelmente_equivalente"], mutant_matrix)

    mark = lambda v: "●" if v else "·"
    report += ["", "## 3. Matriz defeito × abordagem", "",
               md_table (["Defeito", "Origem", "Categoria"] + columns,
                         [row[:3] + [mark (v) for v in row[3:]] for row in fault_matrix]),
               "", f"A matriz dos {len (mutants)} mutantes está em `matriz_mutantes.csv`."]

    # ---- 4. detection per category (faults) and per operator group and file (mutants)
    cat_rows = []

    def category_table (set_name, ids, key_of):
        groups = defaultdict (list)

        for i in ids:
            groups[key_of (i)].append (i)

        rows = []

        for key in sorted (groups, key = lambda g: (-len (groups[g]), g)):
            sub = groups[key]
            counts = [sum (d.detects (i, a) for i in sub) for a in columns]
            cat_rows.extend ([set_name, key, a, k, len (sub)] for a, k in zip (columns, counts))
            rows.append ([key, len (sub)] + [f"{k}/{len (sub)}" for k in counts])

        return rows

    fault_cat = category_table ("defeitos por categoria", faults, lambda i: d.conditions[i]["category"])
    mutant_group = category_table ("mutantes por grupo de operador", mutants, lambda i: mutator_group (d.mutants[i]["mutator"]))
    mutant_file = category_table ("mutantes por arquivo", mutants, lambda i: d.mutants[i]["file"])
    write_csv ("deteccao_por_categoria.csv", ["conjunto", "grupo", "abordagem", "detectados", "n"], cat_rows)

    report += ["", "## 4. Detecção por categoria", "", "Defeitos, por categoria:", "",
               md_table (["Categoria", "n"] + columns, fault_cat),
               "", "Mutantes, por grupo de operador:", "",
               md_table (["Grupo", "n"] + columns, mutant_group),
               "", "Mutantes, por arquivo:", "",
               md_table (["Arquivo", "n"] + columns, mutant_file)]

    # ---- 5. what each approach catches alone
    alone_rows = []

    for set_name, ids in sets[:2]:
        for a in APPROACHES:
            only = [i for i in ids if d.detects (i, a) and not any (d.detects (i, b) for b in APPROACHES if b != a)]
            alone_rows.append ([set_name, a, len (only), ", ".join (only) or "–"])

    write_csv ("deteccao_exclusiva.csv", ["conjunto", "abordagem", "n", "ids"], alone_rows)
    crashing = [i for i in mutants if all (d.detects (i, a) for a in TEST_APPROACHES) and
                any (t["status"] in ("crash", "timeout")
                     for t in map (json.loads, (RAW / i / "run1" / "tests.jsonl").read_text().splitlines())
                     if t["approach"] in TEST_APPROACHES)]
    report += ["", "## 5. Detecção exclusiva", "",
               "Condições que só uma abordagem detecta:", "",
               md_table (["Conjunto", "Abordagem", "n", "Ids"], alone_rows),
               "",
               f"Mutantes que travam ou estouram o tempo em alguma das quatro estratégias e são detectados por todas elas: "
               f"{len (crashing)} de {len (mutants)}."]

    # ---- 6. false alarms
    fa_rows = []

    for cid in harmless:
        runs = len (d.conditions[cid]["runs"])
        fa_rows.append ([cid, runs] + [f"{d.failing_runs (cid, a)}/{runs}" for a in APPROACHES])

    fa_totals = [sum (d.detects (c, a) for c in harmless) for a in APPROACHES]
    write_csv ("alarmes_falsos.csv", ["condicao", "execucoes"] + APPROACHES,
               [[c, len (d.conditions[c]["runs"])] + [d.failing_runs (c, a) for a in APPROACHES] for c in harmless])
    report += ["", "## 6. Alarmes falsos", "",
               "Execuções que falharam em cada condição inofensiva, nas tolerâncias padrão:", "",
               md_table (["Condição", "Execuções"] + APPROACHES, fa_rows),
               "",
               md_table (["", *APPROACHES], [["Condições com alarme falso", *[f"{k}/{len (harmless)}" for k in fa_totals]]])]

    # ---- 7. paired comparison: Cochran's Q and McNemar with Holm
    stat_rows = []
    md_stats = []

    for set_name, ids in sets:
        if ids is mutants_ne:
            # Conditions no approach detects add nothing to Cochran's Q or McNemar, so the results equal the full set
            md_stats += [f"**{set_name}**: idênticos aos de todos os mutantes, porque condições que nenhuma abordagem "
                         "detecta não entram no Q de Cochran nem no McNemar.", ""]
            continue

        matrix = [[d.detects (i, a) for a in APPROACHES] for i in ids]
        q, df, p = cochran_q (matrix)
        stat_rows.append ([set_name, "cochran_q", "todas", round (q, 4) if not math.isnan (q) else "", df, p, "", "", ""])
        md_stats += [f"**{set_name}** (n = {len (ids)}): Q de Cochran = {num (q, 2) if not math.isnan (q) else '–'}, "
                     f"gl = {df}, p = {p_text (p)}.", ""]

        pairs = list (combinations (APPROACHES, 2))
        tests = [mcnemar_exact ([row[APPROACHES.index (a)] for row in matrix],
                                [row[APPROACHES.index (b)] for row in matrix]) for a, b in pairs]
        adjusted = holm ([t[2] for t in tests])
        rows = []

        for (a, b), (only_a, only_b, p_raw), p_adj in zip (pairs, tests, adjusted):
            stat_rows.append ([set_name, "mcnemar_exato", f"{a} x {b}", "", "", p_raw, p_adj, only_a, only_b])
            rows.append ([f"{a} × {b}", only_a, only_b, p_text (p_raw), p_text (p_adj)])

        md_stats += [md_table (["Par", "Só o 1º detecta", "Só o 2º detecta", "p", "p (Holm)"], rows), ""]

        tests_union = [d.detects (i, ANY_TEST) for i in ids]
        only_a, only_b, p_raw = mcnemar_exact (tests_union, [d.detects (i, "pluginval") for i in ids])
        stat_rows.append ([set_name, "mcnemar_exato_complementar", f"{ANY_TEST} x pluginval", "", "", p_raw, "",
                           only_a, only_b])
        md_stats += [f"Complementar (QP2, fora da correção de Holm): {ANY_TEST} × pluginval: só os testes detectam "
                     f"{only_a}, só o pluginval detecta {only_b}, p = {p_text (p_raw)}.", ""]

    write_csv ("estatistica.csv", ["conjunto", "teste", "comparacao", "estatistica", "gl", "p", "p_holm",
                                   "so_primeiro", "so_segundo"], stat_rows)
    report += ["", "## 7. Comparação pareada", "",
               "Teste Q de Cochran entre as cinco abordagens, e McNemar exato par a par com correção de Holm "
               "(10 pares por conjunto).", ""] + md_stats

    # ---- 8. tolerance curve (QP3)
    curve_rows = []
    md_curve = []

    for name, t in d.tolerances.items():
        if len (t["grid"]) < 2:
            continue

        approach = next (a for prefix, a in SWEPT_BY_APPROACH.items() if name.startswith (prefix))
        rows = []

        for g in t["grid"]:
            tol = f"{name}={g:g}"
            counts = { set_name: sum (d.detects (i, approach, tol) for i in ids) for set_name, ids in sets }
            false_alarms = [c for c in harmless if d.detects (c, approach, tol)]
            ref_fails = sum (d.detected[(c, r, approach, tol)] for c in reference for r in d.conditions[c]["runs"])
            is_default = g == t["value"]
            curve_rows.append ([name, t["unit"], t["bound"], approach, g, int (is_default)] +
                               [counts[s] for s, _ in sets] + [len (false_alarms), len (harmless), ref_fails])
            rows.append ([f"{g:g}" + (" (padrão)" if is_default else "")] +
                         [f"{counts[s]} ({pct (counts[s], len (ids))})" for s, ids in sets] +
                         [f"{len (false_alarms)}" + (f" ({', '.join (false_alarms)})" if false_alarms else ""), ref_fails])

        md_curve += [f"**{name}** ({approach}, unidade: {t['unit']}, limite "
                     f"{'máximo' if t['bound'] == 'atMost' else 'mínimo'}):", "",
                     md_table (["Valor", "Defeitos", "Mutantes", "Mutantes não equivalentes", "Alarmes falsos",
                                "Falhas na referência"], rows), ""]

    tolerance_chart (curve_rows, len (faults), len (mutants), len (harmless))
    write_csv ("curva_tolerancia.csv", ["tolerancia", "unidade", "limite", "abordagem", "valor", "padrao",
                                        "defeitos_detectados", "mutantes_detectados", "mutantes_nao_equiv_detectados",
                                        "alarmes_falsos", "condicoes_inofensivas", "falhas_referencia"], curve_rows)

    # A grid point that fails on the clean code detects everything, so it says nothing about a mutant
    usable = [(name, approach, value) for name, _, _, approach, value, _, *rest in curve_rows if rest[-1] == 0]
    revived = defaultdict (list)

    for m in equivalent:
        for name, approach, value in usable:
            if d.detects (m, approach, f"{name}={value:g}"):
                revived[m].append (f"{name}={value:g}")

    revived_text = "; ".join (f"{m} ({', '.join (points)})" for m, points in sorted (revived.items()))
    report += ["", "## 8. Curva de tolerância (QP3)", "",
               "Cada tolerância percorre sua grade com as demais no padrão (seção 6 do protocolo). Os conjuntos têm "
               f"{len (faults)} defeitos, {len (mutants)} mutantes e {len (mutants_ne)} mutantes não equivalentes; "
               f"alarmes falsos contam as {len (harmless)} condições inofensivas. \"Falhas na referência\" conta as "
               "execuções do código limpo no Linux que falhariam naquele ponto. Gráfico: `curva_tolerancia.svg`.",
               ""] + md_curve + [
               f"Dos {len (equivalent)} mutantes possivelmente equivalentes, {len (revived)} são detectados em algum ponto "
               "da grade em que o código limpo passa" + (f": {revived_text}." if revived else ".")]

    # ---- 9. cost
    cost_rows = []
    md_cost = []
    tests_per_approach = defaultdict (int)

    for line in (RAW / "baseline" / "run1" / "tests.jsonl").read_text().splitlines():
        tests_per_approach[json.loads (line)["approach"]] += 1

    shared = count_code_lines (source_files (ROOT / "tests" / "common"))

    for a in APPROACHES:
        times = {}

        for label, cid in (("linux", "baseline"), ("windows", "windows"), ("macos", "macos")):
            values = [d.runtime[(cid, r, a)] for r in sorted (d.conditions[cid]["runs"])]
            times[label] = (statistics.mean (values), statistics.stdev (values) if len (values) > 1 else 0.0)

        if a == "pluginval":
            loc, n_tests = 0, "–"
            loc_text = "0 (ferramenta externa)"
        else:
            loc = count_code_lines (source_files (ROOT / "tests" / TEST_DIRS[a]))
            n_tests = tests_per_approach[a]
            loc_text = str (loc)

        cost_rows.append ([a, n_tests, loc] + [round (x, 3) for label in times for x in times[label]])
        md_cost.append ([a, n_tests, loc_text] + [f"{num (m)} ± {num (s)}" for m, s in times.values()])

    references = sorted ((ROOT / "tests" / "references").glob ("*.wav"))
    write_csv ("custo.csv", ["abordagem", "testes", "linhas_de_codigo", "linux_media_s", "linux_dp_s",
                             "windows_media_s", "windows_dp_s", "macos_media_s", "macos_dp_s"], cost_rows)
    report += ["", "## 9. Custo", "",
               "Tempo de execução de cada abordagem no código limpo (média ± desvio padrão das 5 execuções, em segundos) "
               "e linhas de código dos testes (sem linhas em branco e comentários).", "",
               md_table (["Abordagem", "Testes", "Linhas de código", "Linux (s)", "Windows (s)", "macOS (s)"], md_cost),
               "",
               f"Código compartilhado por todas as estratégias (`tests/common/`): {shared} linhas. O golden master também "
               f"guarda {len (references)} arquivos de referência ({num (sum (p.stat().st_size for p in references) / 1e6)} MB)."]

    (OUT / "RESULTADOS.md").write_text ("\n".join (report) + "\n")
    print (f"{len (faults)} defeitos, {len (mutants)} mutantes ({len (equivalent)} possivelmente equivalentes), "
           f"{len (harmless)} condições inofensivas -> {OUT}")


if __name__ == "__main__":
    main()
