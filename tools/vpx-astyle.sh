#!/bin/bash
set -e
astyle --style=java --indent=spaces=2 --indent-switches\
       --min-conditional-indent=0 \
       --pad-oper --pad-header --unpad-paren \
       --align-pointer=name \
       --indent-preprocessor --convert-tabs --indent-labels \
       --suffix=none --quiet --max-instatement-indent=80 "$@"
sed -i 's/[[:space:]]\{1,\},/,/g' "$@"
sed -i 's/[[:space:]]\{1,\};/;/g' "$@"

# Disabled, too greedy?
#sed -i 's;[[:space:]]\{1,\}\[;[;g' "$@"

sed -i 's/ \{1,\}--;/--;/g' "$@"
sed -i 's/ \{1,\}++;/++;/g' "$@"
sed -i 's/,}/}/g' "$@"
sed -i 's;//\([^/[:space:]].*$\);// \1;g' $@
sed -i 's/^\(public\|private\|protected\):$/ \1:/g' "$@"
sed -i 's/[[:space:]]\{1,\}$//g' "$@"
