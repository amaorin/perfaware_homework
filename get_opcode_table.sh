#!/bin/bash
# gets the x86 opcode table from "https://www.ic.unicamp.br/~celio/mc404/opcodes.html"
'''' &> /dev/null
opcode_table="opcode_table.html"
if [[ ! -e $opcode_table ]]; then
	curl -o $opcode_table "https://www.ic.unicamp.br/~celio/mc404/opcodes.html"
fi
python $0 $opcode_table
exit
'''
import sys

with open(sys.argv[1], "rb") as f:
    opcode_site = f.read()
    opcode_site = opcode_site.replace(b"\r", b"") # the file has CRCRLF line endings....
    opcode_site = opcode_site.replace(b"\x10", b"") # the file has data link escape characters for some reason....
    opcode_site = str(opcode_site, "ascii")

main_instructions_idx = opcode_site.index("Main Instructions")
table_start_idx       = opcode_site.index("<TABLE", main_instructions_idx)
table_end_idx         = opcode_site.index("</TABLE>", table_start_idx) + len("<TABLE/>") - 1

opcode_table_text = opcode_site[table_start_idx:table_end_idx + 1]

opcode_table_text = opcode_table_text[opcode_table_text.index("<TR>"):]

next_row_idx = opcode_table_text.index("<TR>", len("<TR>"))
first_row    = opcode_table_text[:next_row_idx]

opcode_table_text = opcode_table_text[next_row_idx:]
opcode_table_text = opcode_table_text[:opcode_table_text.index("\n\n</TABLE>")]

assert first_row == "<TR>\n<TH>Name\n<TH>Regs\n<TH>Opcode\n<TH>Proc\n<TH>Description\n"

rows = opcode_table_text.split("\n\n")

opcodes = []

all_regs = set()

curr_opcode = []
prev_name = ""
for row in rows:
    parts = row.split("\n")
    assert len(parts) == 6
    assert parts[0] == "<TR>"

    parts = parts[1:]

    for i in range(len(parts)):
        assert parts[i].startswith("<TD>")
        parts[i] = parts[i][len("<TD>"):]

    name, regs, opcode, proc, desc = parts

    if name == "&nbsp;":
        name = prev_name
    elif name.find("<A") != -1:
        name = name[name.find(">")+1:]
        name = name[:name.find("<")]

    if name.endswith(":"):
        name = name[:-1]

    if name != prev_name and len(curr_opcode) != 0:
        opcodes.append(curr_opcode)
        curr_opcode = []

    prev_name = name

    if proc.find("8086") != -1:
        first_byte = opcode[:8]
        rest       = opcode[8:]

        all_regs.add(regs)

        curr_opcode.append([name, regs, first_byte, rest])

alu_opcodes = ["ADD", "ADC", "SUB", "SBB", "CMP", "AND", "XOR", "OR"]
shl_opcodes = ["SHR", "SAR", "SHL", "SAL", "ROR", "ROL", "RCL", "RCR"]
mul_opcodes = ["MUL", "DIV", "IDIV", "IMUL"]
segments    = ["CS", "DS", "ES"]

for encodings in opcodes:
    name = encodings[0][0]
    has_mm = 0
    for enc in encodings:
        if enc[1] in ["Reg,Reg", "Mem,Reg", "Reg,Mem", "RegWord,Mem"]:
            has_mm += 1

    if has_mm > 0:
        print(name, has_mm)

