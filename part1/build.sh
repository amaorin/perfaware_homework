#!/bin/bash

cd $(dirname -- ${BASH_SOURCE[0]})
mkdir -p build
cd build

clang -o sim86 -Wall -Wextra -Wshadow -O0 -ggdb ../sim86.c
