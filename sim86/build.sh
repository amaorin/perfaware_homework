#!/bin/bash

set -e

cd $(dirname -- ${BASH_SOURCE[0]})
mkdir -p build
cd build

clang -O0 -ggdb -o sim86 ../sim86.c
