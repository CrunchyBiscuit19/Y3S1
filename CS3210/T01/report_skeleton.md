<!--
CS3210 Tutorial 1 — Report skeleton (Exercise 15)
Target: <= 2 pages (excluding appendix), A4, reasonable font/margins.
Each numbered section below MUST have a visually distinct header.
Delete these comments and all bracketed [ ... ] prompts before submitting.
Rename the final PDF to your student number, e.g. A0123456X.pdf. Do NOT zip.
-->

# CS3210 Tutorial 1 Report — `asdf.cpp`

**Name:** [Your name]  **Student number:** [A0123456X]  **Date:** [ ]

---

## 1. Overview of the Program

[2–4 sentences, high-level. What does `asdf.cpp` compute? What is the core
data structure and access pattern? Which parameter(s) inside the program are
worth varying, and why? Assume a reader who knows parallel computing and C++ —
explain the *idea*, not every line.]

## 2. Performance vs. Parameter(s)

[State which parameter you varied and over what range, and why this parameter is
the interesting one (e.g. it changes the memory access pattern / cache behavior).]

[Insert the line graph here — e.g. time (or LLC-miss rate) vs. parameter, with
error bars from your >=5 repeats. Reference it as "Figure 1".]

![Figure 1: performance vs. parameter](figure1.png)

**Analysis.** [Don't just describe the trend — explain the *underlying cause*.
Tie the shape of the curve to a hypothesis about hardware behavior (cache line
size, stride vs. locality, TLB, etc.). Back the claim with a supporting metric,
not just wall time.]

## 3. Optimization

**What I changed.** [Describe ONE optimization and the intuition. Do not touch
RNG; no parallelization; no trivial wins like removing prints.]

**Why it should help (hypothesis).** [e.g. improves spatial locality → fewer
LLC misses.]

**Evidence.** [Before vs. after, each measured >=5 times on the same node/config.
Show both time AND the supporting metric (e.g. LLC-load-misses) so the win is
explained, not just observed. Reference "Figure 2" / a small table.]

![Figure 2: before vs. after](figure2.png)

| Version | Array size | Mean time (s) | Std dev | LLC-miss rate |
|---|---|---|---|---|
| Original | [N] | [ ] | [ ] | [ ] |
| Optimized | [N] | [ ] | [ ] | [ ] |

**Discussion.** [Did it work? For which array sizes? Justify the array size you
chose (large enough that the effect dominates noise). If it didn't generalize,
say so and hypothesize why.]

---

## Appendix (does not count toward the 2 pages)

### A. Reproduction details
- **Cluster/Slurm:** results collected via Slurm on the CS3210 PDC lab cluster.
- **Node / partition:** [e.g. `soctf-pdc-001`, partition `xs-4114`, Intel Xeon
  Silver 4114 — from `lscpu` / `sinfo`]
- **Compiler + flags:** [`g++ -g -O2 ...`, version]
- **Program inputs / parameter values:** [strides / array sizes tested]
- **Timing method:** [`perf stat -r 5 -e ... ` / `/usr/bin/time -v`]
- **Repeats per data point:** [>=5]

### B. Raw measurement data
[Full table of every run, or a link to an external sheet — e.g. Google Sheets.
This must let a TA verify the graphs independently.]

| param | run | cycles | instructions | llc_loads | llc_misses | seconds |
|---|---|---|---|---|---|---|
| ... | ... | ... | ... | ... | ... | ... |

### C. Key code excerpt(s)
[Only the important parts you changed — not the whole file.]
