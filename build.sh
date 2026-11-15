#!/bin/bash

set -e

cd $(dirname -- ${BASH_SOURCE[0]})
mkdir -p build
cd build

WARN_OPTS=(-Wall -Wextra -Wshadow -Wno-unused-parameter -Wno-bitwise-instead-of-logical)
DEBUG_OPTS=(-O0 -ggdb)
COMMON_OPTS=(${WARN_OPTS[@]} ${DEBUG_OPTS[@]})

clang ${COMMON_OPTS[@]} -lm -o haversine_generator ../src/haversine_generator.c
