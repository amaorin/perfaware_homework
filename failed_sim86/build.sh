#!/bin/bash

set -e

cd $(dirname -- ${BASH_SOURCE[0]})
mkdir -p build
cd build

clang -O0 -ggdb -o sim8086 ../src/sim8086.c
