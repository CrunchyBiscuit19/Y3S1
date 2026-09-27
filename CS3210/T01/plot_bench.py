import csv
import math
import re
from pathlib import Path
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

BASE = Path(__file__).resolve().parent / "T01"
OUT_CSV = Path(__file__).resolve().parent / "all_runs.csv"
PLOT_DIR = Path(__file__).resolve().parent / "plots"

HARDWARE = ["xs-4114", "i7-7700", "i7-13700"]
HW_COLORS = {"xs-4114": "#1f77b4", "i7-7700": "#ff7f0e", "i7-13700": "#2ca02c"}

ELEMS_PER_UNIT = 10000
BYTES_PER_ELEM = 4
L2_SIZES = {
    "xs-4114": 1 * 1024 * 1024,
    "i7-7700": 256 * 1024,
    "i7-13700": 2 * 1024 * 1024,
}

CANON = ["task_clock_msec", "cycles", "instructions", "branch_misses",
         "cache_references", "cache_misses",
         "l1_dcache_loads", "l1_dcache_load_misses",
         "l2_references", "l2_misses",
         "llc_loads", "llc_load_misses"]

METRICS = ["task_clock_msec", "cycles", "instructions",
           "branch_misses", "cache_references", "cache_misses"]
METRIC_LABELS = {
    "task_clock_msec": "Task clock (ms)", "cycles": "CPU cycles",
    "instructions": "Instructions", "branch_misses": "Branch misses",
    "cache_references": "Cache references", "cache_misses": "Cache misses",
}

KEY_COLS = ("bin", "size", "run")
BASE_RE = re.compile(r"^runs_(?!cache_)(.+)\.csv$")
CACHE_RE = re.compile(r"^runs_cache_(.+)\.csv$")


def _num(x: str) -> float:
    try:
        return float(x)
    except (TypeError, ValueError):
        return math.nan


def read_runs(path: Path, hardware: str):
    with path.open(newline="") as f:
        reader = csv.reader(f)
        header = next(reader, None)
        if not header:
            return
        header = [h.strip() for h in header]
        for fields in reader:
            if not fields:
                continue
            row = dict(zip(header, [x.strip() for x in fields]))
            if row.get("run", "").lower() == "avg":
                continue
            try:
                size = int(row["size"])
                run = int(row["run"])
            except (KeyError, ValueError):
                continue
            rec = {
                "binary": row["bin"].replace("_asdf", ""),
                "hardware": hardware,
                "size": size,
                "array_size": size * ELEMS_PER_UNIT,
                "run": run,
            }
            for col, val in row.items():
                if col not in KEY_COLS:
                    rec[col] = _num(val)
            yield rec


def consolidate() -> pd.DataFrame:
    records = []
    for csv_path in sorted(BASE.glob("runs_*.csv")):
        m = CACHE_RE.match(csv_path.name) or BASE_RE.match(csv_path.name)
        if not m:
            continue
        hardware = m.group(1)
        records.extend(read_runs(csv_path, hardware))

    df = pd.DataFrame.from_records(records)
    id_cols = ["binary", "hardware", "size", "array_size", "run"]
    metric_cols = ([c for c in CANON if c in df.columns]
                   + [c for c in df.columns if c not in CANON and c not in id_cols])
    df = df[id_cols + metric_cols]
    df = df.sort_values(
        ["binary", "hardware", "array_size", "run"]).reset_index(drop=True)
    df.to_csv(OUT_CSV, index=False)
    print(f"Wrote {len(df)} rows, columns={metric_cols} -> {OUT_CSV}")
    return df


def usable(df: pd.DataFrame, *cols) -> bool:
    return all(c in df.columns for c in cols) and \
        bool(df[list(cols)].notna().all(axis=1).any())


def plot_cache_levels(df: pd.DataFrame):
    PLOT_DIR.mkdir(exist_ok=True)
    df = df.copy()

    plots = []  
    if usable(df, "l1_dcache_load_misses", "l1_dcache_loads"):
        df["l1_miss_rate"] = df["l1_dcache_load_misses"] / df["l1_dcache_loads"]
        plots.append(("l1_miss_rate", "L1D load miss rate", False))
    if usable(df, "l2_misses", "l2_references"):
        df["l2_miss_rate"] = df["l2_misses"] / df["l2_references"]
        plots.append(("l2_miss_rate", "L2 miss rate", False))
        plots.append(("l2_misses", "L2 misses (count)", True))
    if usable(df, "llc_load_misses", "llc_loads"):
        df["llc_miss_rate"] = df["llc_load_misses"] / df["llc_loads"]
        plots.append(("llc_miss_rate", "LLC load miss rate", False))

    made = []
    for binary in sorted(df["binary"].unique()):
        bdf = df[df["binary"] == binary]
        for col, label, log_y in plots:
            if col not in bdf or bdf[col].notna().sum() == 0:
                continue
            agg = bdf.groupby(["hardware", "array_size"])[col].agg(["mean", "std"])
            xs = np.sort(bdf["array_size"].unique())
            fig, ax = plt.subplots(figsize=(7, 4.5))
            for hw in HARDWARE:
                if hw not in agg.index.get_level_values(0):
                    continue
                s = agg.loc[hw]
                ax.errorbar(s.index.values, s["mean"].values,
                            yerr=np.nan_to_num(s["std"].values),
                            marker="o", capsize=3, color=HW_COLORS.get(hw), label=hw)
                if hw in L2_SIZES:
                    l2_elems = L2_SIZES[hw] / BYTES_PER_ELEM
                    if xs.min() <= l2_elems <= xs.max():
                        ax.axvline(l2_elems, color=HW_COLORS.get(hw),
                                   ls="--", alpha=0.4)
            ax.set_xlim(xs.min(), xs.max())
            if log_y:
                ax.set_yscale("log")
            ax.set_title(f"{label} vs array size ({binary}_asdf)")
            ax.set_xlabel("Array size (elements)")
            ax.set_ylabel(label)
            ax.grid(True, which="both", ls=":", alpha=0.6)
            ax.legend(title="Hardware (dashed = its L2 size)")
            fig.tight_layout()
            out = PLOT_DIR / f"{binary}_{col}_vs_elements.png"
            fig.savefig(out, dpi=150)
            plt.close(fig)
            made.append(out.name)
            print(f"ploted {out.name}")
    return made


def plot(df: pd.DataFrame):
    PLOT_DIR.mkdir(exist_ok=True)
    present = [m for m in METRICS if m in df.columns]
    agg = df.groupby(["binary", "hardware", "array_size"])[present].agg(["mean", "std"])
    made = []
    for binary in sorted(df["binary"].unique()):
        for metric in present:
            fig, ax = plt.subplots(figsize=(7, 4.5))
            for hw in HARDWARE:
                if (binary, hw) not in agg.index.droplevel(2).unique():
                    continue
                sub = agg.loc[binary].loc[hw]
                ax.errorbar(sub.index.values, sub[(metric, "mean")].values,
                            yerr=np.nan_to_num(sub[(metric, "std")].values),
                            marker="o", capsize=3, color=HW_COLORS.get(hw), label=hw)
            ax.set_title(f"{METRIC_LABELS[metric]} vs array size ({binary}_asdf)")
            ax.set_xlabel("Array size (elements)")
            ax.set_ylabel(METRIC_LABELS[metric])
            ax.grid(True, which="both", ls=":", alpha=0.6)
            ax.legend(title="Hardware")
            fig.tight_layout()
            out = PLOT_DIR / f"{binary}_{metric}.png"
            fig.savefig(out, dpi=150)
            plt.close(fig)
            made.append(out.name)
            print(f"ploted {out.name}")
    return made


if __name__ == "__main__":
    df = consolidate()
    base = plot(df)
    cache = plot_cache_levels(df)
