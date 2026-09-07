# CS3210 Tutorial 1 — Summary: Performance Instrumentation, Slurm & Technical Report Writing

Consolidated from `tutorial01.pdf` and `tutorial01_extra.pdf` (2026/27 Sem 1),
focused on what is needed for the graded report.

## Exercise 15

- Study the given `asdf.cpp` program and write a report, **≤ 2 pages**, A4, reasonable
  font/margins, with **visually distinct headers** for each required section.
- Required sections:
  1. **Overview** of what the code does.
  2. **Description + visualization + data** of how/why performance changes as you vary
     parameter(s) in the program.
  3. **One optimization** you tried (must run faster than the original for sufficiently
     large arrays; justify your array size), with supporting measurements + a hypothesis
     on why it did/didn't work.
     - Do **not** change the random-number generation, do **not** parallelize, do **not**
       count trivial wins (e.g. removing prints), and you don't need the *optimal*
       solution — just something provably faster.
  4. **Appendix** (doesn't count toward 2 pages): exact reproduction details (node,
     inputs, timing method) + raw performance data (table or link to e.g. Google Sheets).
     **Results must be collected via Slurm on the CS3210 lab cluster.**
- Don't dump full code, only the important parts.
- **Submit to Canvas as a PDF named `<student_number>.pdf`** (e.g. `A0123456X.pdf`),
  not zipped. Deadline: **Monday 31 August, 2 pm** (not 11:59 pm).

## Good Technical Writing

- **Audience:** Solid parallel-computing + programming background who wants to understand your approach. 
- Explain the high-level overview + only the important details. 
- Avoid excessive implementation detail.
- **Proving an optimization works**
  * **Repeated measurements** before and after each optimization (≥ 5 runs each — a 2-point before/after is *not* sufficient).
  * Test under **different parameters/conditions** to show it generalizes.
  * Collect **supporting metrics** (eg. if claim better cache behavior, show L1/L2/L3 miss data).
  * Make sure runs are long/large enough that signal outweighs noise.
- Hypothesize the *underlying causes*. Prefer **deep exploration of a few parameters** over shallow coverage of every combination.

## Tools you can use

| Tool                          | Purpose                                                                       | Example                                                                 |
| ----------------------------- | ----------------------------------------------------------------------------- | ----------------------------------------------------------------------- |
| `/usr/bin/time`               | wall/user/system time, basic run stats                                        | `/usr/bin/time -v ./sum_64`                                             |
| `perf stat`                   | hardware event counters (cycles, instructions, cache misses)                  | `perf stat -e LLC-loads,LLC-load-misses,cycles,instructions -- ./sum_1` |
| `perf record` → `perf report` | attribute cycles/cache-misses to functions & source lines (compile with `-g`) | `perf record -e LLC-load-misses -- ./sum_1`                             |
| `flamegraph` (extra)          | visualize call-stack hotspots → `flamegraph.svg`                              | `flamegraph -- ./sum_1`                                                 |
| `lscpu` / `lstopo`            | inspect node hardware (sockets, cores/threads, cache sizes)                   | `lstopo --of ascii`                                                     |
| Grafana                       | per-node load/session/CPU/mem monitoring                                      | `https://grafana.cs3210.org` (needs NUS VPN)                            |

* Useful `perf` events: `LLC-loads`, `LLC-load-misses`, `cycles`, `instructions`,
`L1-dcache-load-misses`, `L1-dcache-loads`, `L1-dcache-stores`. 
* Repeat a measurement N times with `perf stat -r 3 <cmd>`
* List events with `perf list`.

## Parameters to test

Vary a *few* meaningful parameters deeply:
- Input parameters
- Node classes (Xeon Silver 4114, i7-7700, i7-9700, Xeon W-2245, W5-3425, i7-13700) via Slurm partitions.
- Machine configuration, number of threads.

## Copy-Paste

```bash
g++ -g -O3 -o asdf asdf.cpp
```
```bash
#!/bin/bash
#SBATCH --job-name=asdf
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --partition=xs-4114
#SBATCH --exclusive
#SBATCH --time=00:05:00
#SBATCH --output=asdf_%j.log
#SBATCH --error=asdf_%j.log

# sweep a parameter (e.g. stride) and repeat each 5x, appending to a CSV
# TODO
```

```bash
sbatch run.sh
```
```bash
watch -n 0.5 squeue
```

```bash
perf stat -r 5 -e cycles,instructions,LLC-loads,LLC-load-misses ./asdf 64 2>&1 | tee asdf_stride64.txt
```

> Note: `perf`/`time` write stats to **stderr**, so redirect with `2>` / `2>&1`, not just `>`. `-x,` makes `perf stat` emit CSV that's easy to parse for graphing.

Generate a flamegraph (produces `flamegraph.svg` in the cwd):

```bash
flamegraph -- ./asdf 64
```

## Getting files back to your machine (scp)

```bash
scp e1398738@soctf-pdc-001.comp.nus.edu.sg:~/T01/flamegraph.svg .
```
```bash
scp asdf.cpp e1398738@soctf-pdc-001.comp.nus.edu.sg:~/T01/asdf.cpp
```

## Graph / visualization

- Prefer a line graph, don't dump everything into a table, and no `perf` screenshots as visualization.
- Always include the raw measurement data
- Every graph must have title, axis labels, axis markings/numbering, clear data
  points.
- Add when applicable gridlines, error bars (from your ≥5 repeats — min/max, std dev, etc.), trendlines (include R² if you fit a line/curve).

## Cluster / Slurm essentials (context)

- Nodes are grouped into **partitions by hardware type** (`sinfo` to list). The `*` partition is the default. 
  Slurm for exclusive, accurate measurements.
- `sbatch` = batch/non-interactive (preferred for reproducible runs). Monitor with `squeue`
- CPUs-allocated with `sacct`. `--exclusive` ensures no noise from other jobs.
- Target a specific partition (`--partition xs-4114`). 
- Use `lscpu`/`lstopo` to record the exact config in your appendix.

### Node hostnames → hardware

| Hostnames             | CPU                                |
| --------------------- | ---------------------------------- |
| `soctf-pdc-001`–`008` | Intel Xeon Silver 4114             |
| `soctf-pdc-009`–`016` | Intel Core i7-7700                 |
| `soctf-pdc-018`–`019` | Dual-socket Intel Xeon Silver 4114 |
| `soctf-pdc-020`–`021` | Intel Core i7-9700                 |
| `soctf-pdc-022`–`024` | Intel Xeon W-2245                  |
| `soctf-pdc-025`–`032` | Intel Xeon W5-3425                 |
| `soctf-pdc-033`–`040` | Intel Core i7-13700                |

## Benchmark

```bash
# choose one in 'idle'
sinfo -p xs-4114 -o "%n %t %C"   
sinfo -p i7-7700 -o "%n %t %C"   
sinfo -p i7-13700 -o "%n %t %C"

sbatch --partition=xs-4114 bench.sh
sbatch --partition=i7-7700 bench.sh
sbatch --partition=i7-13700 bench.sh
```
Monitor:

```bash
watch -n 0.5 squeue --me
```

Outputs (pull back with the `scp` commands above):
- `runs_slow_asdf_<partition>.csv`, `runs_fast_asdf_<partition>.csv` — 5 per-run rows + an `avg` row, per binary per hardware type.
- `flame_<bin>_<partition>/flamegraph.svg` — call-stack hotspots per binary per hardware type.

> Keep `slow_asdf`'s array size small enough that 5 bubble-sort runs finish inside
> `--time`; bubble sort is O(n²), so 1e7 will not complete. Use `-r 3` for the slow binary
> if needed.
