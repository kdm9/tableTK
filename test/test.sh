#!/bin/bash
set -x
set -e
valgrind --leak-check=full bin/filterTable  -r 1 -c 1 -i data/small.tab -z 2 > data/out.tab
md5sum data/out.tab
echo "sum should be ''"
