#!/bin/sh

if [ $# -ne 2 ]; then
  echo "Usage: $0 <filename.yuv> WxH" 1>&2
  exit 1
fi
FILE="$1"
DIM="$2"
PROCESSES=16 # Parallelism

tiny_var "$FILE" "$DIM"

echo "$FILE".var-y*dat | xargs -P "$PROCESSES" -n 1 ./plot_vars.R
