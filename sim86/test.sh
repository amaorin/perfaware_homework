#!/bin/bash

set -e

cd $(dirname -- ${BASH_SOURCE[0]})
mkdir -p build
cd build

"../build.sh"

rm -rf ./test_dir*

COMPUTER_ENHANCE_HOME=$(cd "../../../computer_enhance" && pwd)
PERFAWARE_HOME="$COMPUTER_ENHANCE_HOME/perfaware"

SIM8086_DECODE_TEST_FILES=(
#listing_0037_single_register_mov
#listing_0038_many_register_mov
#listing_0039_more_movs
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

SIM8086_EXEC_TEST_FILES=(
#listing_0043_immediate_movs
#listing_0044_register_movs
#listing_0045_challenge_register_movs
#listing_0046_add_sub_cmp
#listing_0047_challenge_flags
#listing_0048_ip_register
#listing_0049_conditional_jumps
#listing_0050_challenge_jumps
)

function MaxLen
{
	local result=0

	for string in $@; do
		local len=${#string}
		if (( len > result )); then
			result=$len
		fi
	done

	echo $result
}

max_decode_name_len=$(MaxLen ${SIM8086_DECODE_TEST_FILES[@]})
max_exec_name_len=$(MaxLen ${SIM8086_EXEC_TEST_FILES[@]})

max_name_len=$((max_decode_name_len > max_exec_name_len ? max_decode_name_len : max_exec_name_len))

test_name_col_size=$((max_name_len+3))

test_dir="test_dir-$(uuidgen)"

mkdir $test_dir
cd $test_dir

function PrintTestName
{
	local index=$1
	local count=$2
	local name=$3

	printf "[%02d/%02d] %s" $(($index+1)) $count $name
	local line_len=${#name}
	while (( line_len < test_name_col_size)); do
		echo -n .
		line_len=$((line_len+1))
	done
}

echo
echo " Testing sim8086 decode |"
echo "------------------------+"
echo
for index in ${!SIM8086_DECODE_TEST_FILES[@]}; do
	test_file=${SIM8086_DECODE_TEST_FILES[index]}
	PrintTestName $index ${#SIM8086_DECODE_TEST_FILES[@]} $test_file

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

echo
echo " Testing sim8086 exec   |"
echo "------------------------+"
echo
for index in ${!SIM8086_EXEC_TEST_FILES[@]}; do
	test_file=${SIM8086_EXEC_TEST_FILES[index]}
	PrintTestName $index ${#SIM8086_EXEC_TEST_FILES[@]} $test_file

	cp $PERFAWARE_HOME/part1/$test_file.asm ./$test_file.asm
	cp $PERFAWARE_HOME/part1/$test_file.txt ./$test_file.txt
	nasm -o $test_file ./$test_file.asm

	"../sim8086" --exec ./$test_file > output.txt

	ref_regs=$(sed "1,/Final registers/d" $test_file.txt)
	out_regs=$(sed "1,/Final registers/d" output.txt)

	if cmp -s <(echo $ref_regs) <(echo $out_regs); then
		printf "\033[0;32mpassed\033[0;0m\n"
	else
		printf "\033[0;31mfailed\033[0;0m\n"

		exit 1
	fi
done

printf "\nAll %02d tests passed\n\n" $((${#SIM8086_DECODE_TEST_FILES[@]} + ${#SIM8086_EXEC_TEST_FILES[@]}))
