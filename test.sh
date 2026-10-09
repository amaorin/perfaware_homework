#!/bin/bash

set -e

cd $(dirname -- ${BASH_SOURCE[0]})
mkdir -p build
cd build

COMPUTER_ENHANCE_HOME=$(cd "../../computer_enhance" && pwd)
PERFAWARE_HOME="$COMPUTER_ENHANCE_HOME/perfaware"

SIM8086_DECODE_TEST_FILES=(
	listing_0037_single_register_mov
	listing_0038_many_register_mov
)

test_dir=$(uuidgen)

mkdir $test_dir
cd $test_dir

for test_file in ${SIM8086_DECODE_TEST_FILES[@]}; do
	cp $PERFAWARE_HOME/part1/$test_file.asm ./$test_file.asm
	nasm -o $test_file ./$test_file.asm
	"../sim8086" ./$test_file > output.asm
	nasm -o output ./output.asm

	if cmp -s "./$test_file" "./output"; then
		echo passed
	else
		xxd $test_file > $test_file.hex
		xxd output     > output.hex
	fi
done

cd ..
#rm $test_dir/listing*
#rm $test_dir/output*
#rmdir $test_dir
