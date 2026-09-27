#!/bin/bash
#SBATCH --job-name=asdf_bench
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --exclusive
#SBATCH --time=01:00:00
#SBATCH --output=bench_%j.log
#SBATCH --error=bench_%j.log

PART="${SLURM_JOB_PARTITION:?submit with a partition, e.g. sbatch -p xs-4114 bench.sh}"
EVENTS=task-clock,cycles,instructions,branch-misses,cache-references,cache-misses
HEADER="bin,size,run,task_clock_msec,cycles,instructions,branch_misses,cache_references,cache_misses"
BINARIES="slow_asdf fast_asdf"
SIZES="1 2 4 6 8 10"
RUNS=5

csv="runs_${PART}.csv"
echo "$HEADER" > "$csv"
for bin in $BINARIES; do
  for size in $SIZES; do
    for run in $(seq 1 $RUNS); do
      for attempt in 1 2 3; do
        out=$(srun perf stat -x, -e "$EVENTS" "./$bin" "$size" 2>&1 >/dev/null)
        vals=$(echo "$out" | awk -F, -v ev="$EVENTS" '
          BEGIN{n=split(ev,a,",")}
          $3!=""{e=$3; sub(/^cpu_[a-z]+\//,"",e); sub(/\/$/,"",e);
                 if(!(e in v) || (v[e] ~ /[^0-9.]/ && $1 ~ /^[0-9.]+$/)) v[e]=$1}
          END{for(i=1;i<=n;i++){x=(a[i] in v)?v[a[i]]:"NA"; if(x ~ /[^0-9.]/)x="NA"; printf "%s%s", (i>1?",":""), x}}')
        case "$vals" in *NA*) continue;; esac
        break
      done
      echo "${bin},${size},${run},${vals}" >> "$csv"
    done
    awk -F, -v b="$bin" -v sz="$size" 'NR>1 && $1==b && $2==sz{for(i=4;i<=NF;i++)s[i]+=$i;n++;c=NF} END{printf "%s,%s,avg",b,sz; for(i=4;i<=c;i++)printf ",%.2f",s[i]/n; print ""}' "$csv" >> "$csv"
  done
  srun bash -c 'd="flame_'"$bin"'_'"$PART"'"; mkdir -p "$d"; cd "$d"; flamegraph -- "../'"$bin"'" 10'
done
echo "Done on $PART. Collect runs_${PART}.csv and flame_*_*/flamegraph.svg"