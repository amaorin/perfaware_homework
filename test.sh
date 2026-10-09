#!/bin/bash

set -e

COMPUTER_ENHANCE_HOME="../../computer_enhance"
PERFAWARE_HOME="$COMPUTER_ENHANCE_HOME/perfaware"

cd $(dirname -- ${BASH_SOURCE[0]})
mkdir -p build
cd build

SIM8086_DECODE_TEST_FILES=(
	listing_0037_single_register_mov
	listing_0038_many_register_mov
	listing_0039_more_movs
	listing_0040_challenge_movs
)

for test_file in ${SIM8086_DECODE_TEST_FILES[@]}; do
	input_asm=$PERFAWARE_HOME/part1/$test_file.asm
	nasm $input_asm
	exec "./sim8086" $test_file > sim8086_output.asm
	nasm sim8086_output.asm
	xxd $test_file > test_file.hex
	xxd sim8086_output > sim8086_output.hex
done
