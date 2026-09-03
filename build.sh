#!/bin/bash

cd $(dirname -- ${BASH_SOURCE[0]})

mkdir -p build
cd build

warn_opts=(-Wall -Wextra -Wshadow)
common_compile_opts=(${warn_opts[@]} -mavx2)
debug_compile_opts=(-Od -g)

clang ${common_compile_opts[@]} -lm -o haversine_gen ../src/haversine_gen.c
