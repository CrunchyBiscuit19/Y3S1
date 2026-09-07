#!/usr/bin/env python3
"""
Combine each hardware's base and cache benchmark CSVs into one file.

For every T01/runs_<hw>.csv + T01/runs_cache_<hw>.csv pair, merge the per-run rows
on (bin, size, run) and write T01/runs_combined_<hw>.csv with all metric columns.
Execution time (task_clock_msec) is taken from runs_<hw>.csv; the cache file's own
task_clock_msec column is dropped. A recomputed 'avg' row follows each (bin, size)
block, matching the layout of the source files. Non-numeric cells ('NA',
'<not counted>') become blank in the output and are skipped when averaging.
"""

import sys
from pathlib import Path

import pandas as pd

BASE = Path(__file__).resolve().parent / "T01"

KEYS = ["bin", "size", "run"]
# Preferred output order; any extra columns are appended in first-seen order.
CANON = ["task_clock_msec", "cycles", "instructions", "branch_misses",
         "cache_references", "cache_misses",
         "l1_dcache_loads", "l1_dcache_load_misses",
         "l2_references", "l2_misses",
         "llc_loads", "llc_load_misses"]


def load(path: Path) -> pd.DataFrame:
    """Read a runs CSV, drop 'avg' rows, coerce metric cells to numbers."""
    df = pd.read_csv(path, dtype=str)
    df = df[df["run"].str.lower() != "avg"].copy()
    df["size"] = df["size"].astype(int)
    df["run"] = df["run"].astype(int)
    for c in df.columns:
        if c not in ("bin",) + tuple(KEYS[1:]):
            df[c] = pd.to_numeric(df[c], errors="coerce")   # NA/<not counted> -> NaN
    return df


def combine(hw: str, base_path: Path, cache_path: Path) -> Path:
    base = load(base_path)
    cache = load(cache_path).drop(columns=["task_clock_msec"], errors="ignore")

    merged = base.merge(cache, on=KEYS, how="outer")

    metrics = [c for c in CANON if c in merged.columns] + \
              [c for c in merged.columns if c not in CANON and c not in KEYS]
    merged = merged[KEYS + metrics].sort_values(["bin", "size", "run"])

    # Rebuild per-(bin,size) blocks, each followed by an 'avg' row.
    blocks = []
    for (b, sz), grp in merged.groupby(["bin", "size"], sort=True):
        blocks.append(grp)
        avg = {"bin": b, "size": sz, "run": "avg"}
        for m in metrics:
            avg[m] = round(grp[m].mean(), 2) if grp[m].notna().any() else ""
        blocks.append(pd.DataFrame([avg]))
    out_df = pd.concat(blocks, ignore_index=True)

    out_path = BASE / f"runs_combined_{hw}.csv"
    out_df.to_csv(out_path, index=False)
    n_runs = len(merged)
    print(f"{hw}: {n_runs} runs -> {out_path.name}")
    return out_path


def main() -> None:
    pairs = []
    for base_path in sorted(BASE.glob("runs_*.csv")):
        name = base_path.name
        if name.startswith("runs_cache_") or name.startswith("runs_combined_"):
            continue
        hw = name[len("runs_"):-len(".csv")]
        cache_path = BASE / f"runs_cache_{hw}.csv"
        if not cache_path.exists():
            print(f"skip {hw}: no {cache_path.name}", file=sys.stderr)
            continue
        pairs.append((hw, base_path, cache_path))

    if not pairs:
        raise SystemExit(f"No runs_<hw>.csv / runs_cache_<hw>.csv pairs under {BASE}")

    for hw, base_path, cache_path in pairs:
        combine(hw, base_path, cache_path)


if __name__ == "__main__":
    main()
