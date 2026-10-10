#!/bin/bash

set -e

cd $(dirname -- ${BASH_SOURCE[0]})
mkdir -p build
cd build

"../build.sh"

rm -rf ./test_dir*

COMPUTER_ENHANCE_HOME=$(cd "../../computer_enhance" && pwd)
PERFAWARE_HOME="$COMPUTER_ENHANCE_HOME/perfaware"

SIM8086_DECODE_TEST_FILES=(
	listing_0037_single_register_mov
	listing_0038_many_register_mov
	listing_0039_more_movs
#listing_0040_challenge_movs
#listing_0041_add_sub_cmp_jnz
#listing_0042_completionist_decode
#listing_0043_immediate_movs
#listing_0044_register_movs
#listing_0045_challenge_register_movs
#listing_0046_add_sub_cmp
#listing_0047_challenge_flags
#listing_0048_ip_register
#listing_0049_conditional_jumps
#listing_0050_challenge_jumps
)

test_name_col_size=0
for test_file in ${SIM8086_DECODE_TEST_FILES[@]}; do
	test_file_len=${#test_file}
	if (( test_file_len > test_name_col_size )); then
		test_name_col_size=$test_file_len
	fi
done

test_name_col_size=$((test_name_col_size+3))

test_dir="test_dir-$(uuidgen)"

mkdir $test_dir
cd $test_dir

echo
echo " Testing sim8086 decode |"
echo "------------------------+"
echo
for index in ${!SIM8086_DECODE_TEST_FILES[@]}; do
	test_file=${SIM8086_DECODE_TEST_FILES[index]}
	printf "[%02d/%02d] %s" $index ${#SIM8086_DECODE_TEST_FILES[@]} $test_file
	line_len=${#test_file}
	while (( line_len < test_name_col_size)); do
		echo -n .
		line_len=$((line_len+1))
	done

	cp $PERFAWARE_HOME/part1/$test_file.asm ./$test_file.asm
	nasm -o $test_file ./$test_file.asm
	"../sim8086" ./$test_file > output.asm
	nasm -o output ./output.asm

	if cmp -s "./$test_file" "./output"; then
		printf "\033[0;32mpassed\033[0;0m\n"
	else
		printf "\033[0;31mfailed\033[0;0m\n"

		xxd $test_file > $test_file.hex
		xxd output     > output.hex

		exit 1
	fi
done

printf "\nAll %02d tests passed\n\n" ${#SIM8086_DECODE_TEST_FILES[@]}

cd ..
rm $test_dir/listing*
rm $test_dir/output*
rmdir $test_dir
