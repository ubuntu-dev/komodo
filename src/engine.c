#include <string.h>
#include <stdio.h>

#include "engine.h"

#define MAX_LEN 128

char thumb_instruction[MAX_LEN];

typedef char *(*disassemble_func)(uint16_t);

static char *disassemble_format1(uint16_t opcode);
static char *disassemble_format2(uint16_t opcode);
static char *disassemble_format3(uint16_t opcode);
static char *disassemble_format4_5_decider(uint16_t opcode);
static char *disassemble_format4(uint16_t opcode);
static char *disassemble_format5(uint16_t opcode);
static char *disassemble_format6(uint16_t opcode);
static char *disassemble_format7_8_decider(uint16_t opcode);
static char *disassemble_format7(uint16_t opcode);
static char *disassemble_format8(uint16_t opcode);
static char *disassemble_format9(uint16_t opcode);
static char *disassemble_format10(uint16_t opcode);
static char *disassemble_format11(uint16_t opcode);
static char *disassemble_format12(uint16_t opcode);
static char *disassemble_format13_14_decider(uint16_t opcode);
static char *disassemble_format13(uint16_t opcode);
static char *disassemble_format14(uint16_t opcode);
static char *disassemble_format15(uint16_t opcode);
static char *disassemble_format16_17_decider(uint16_t opcode);
static char *disassemble_format16(uint16_t opcode);
static char *disassemble_format17(uint16_t opcode);
static char *disassemble_format18(uint16_t opcode);
static char *disassemble_format19(uint16_t opcode);

static uint16_t isolate_bits(uint16_t src, uint8_t start_bit, uint8_t end_bit);

// refer to "Format Summary" on page 5-2 in Thumb Instruction Set document,
// which is also available in this repo under doc/thumb-instruction-set.pdf
disassemble_func disassembler_vector[19];

void engine_init()
{
	// format 1
	disassembler_vector[0] = disassemble_format1;
	disassembler_vector[1] = disassemble_format1;
	disassembler_vector[2] = disassemble_format1;

	// format 2
	disassembler_vector[3] = disassemble_format2;

	// format 3
	disassembler_vector[4] = disassemble_format3;
	disassembler_vector[5] = disassemble_format3;
	disassembler_vector[6] = disassemble_format3;
	disassembler_vector[7] = disassemble_format3;

	// formats 4 and 5
	disassembler_vector[8] = disassemble_format4_5_decider;

	// format 6
	disassembler_vector[9] = disassemble_format6;

	// formats 7 and 8
	disassembler_vector[10] = disassemble_format7_8_decider;
	disassembler_vector[11] = disassemble_format7_8_decider;

	// format 9
	disassembler_vector[12] = disassemble_format9;
	disassembler_vector[13] = disassemble_format9;
	disassembler_vector[14] = disassemble_format9;
	disassembler_vector[15] = disassemble_format9;

	// format 10
	disassembler_vector[16] = disassemble_format10;
	disassembler_vector[17] = disassemble_format10;

	// format 11
	disassembler_vector[18] = disassemble_format11;
	disassembler_vector[19] = disassemble_format11;

	// format 12
	disassembler_vector[20] = disassemble_format12;
	disassembler_vector[21] = disassemble_format12;

	// format 13 and 14
	disassembler_vector[22] = disassemble_format13_14_decider;
	disassembler_vector[23] = disassemble_format13_14_decider;

	// format 15
	disassembler_vector[24] = disassemble_format15;
	disassembler_vector[25] = disassemble_format15;

	// format 16
	disassembler_vector[26] = disassemble_format16;
	// formats 16 and 17
	disassembler_vector[27] = disassemble_format16_17_decider;

	// format 18
	disassembler_vector[28] = disassemble_format18;

	// TODO: index 29 is invalid instruction
	
	// format 19
	disassembler_vector[30] = disassemble_format19;
	disassembler_vector[31] = disassemble_format19;
}

char *engine_get_assembly(uint16_t opcode)
{
	uint8_t opcode_identifier;
	// get five most significant bits of opcode
	opcode_identifier = isolate_bits(opcode, 11, 15);
		// ((opcode & 0xF800) >> 11);

	// index into disassembler_vector
	return disassembler_vector[opcode_identifier](opcode);
}

static char *disassemble_format1(uint16_t opcode)
{
	uint16_t hold;
	int i = 0;

	memset(thumb_instruction, 0, MAX_LEN);
	// isolate opertion bits 11 and 12
	hold = isolate_bits(opcode, 11, 12);
		// ((opcode & 0x1800) >> 11);
	switch(hold){
		case 0:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "LSL");
			break;
		case 1:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "LSR");
			break;
		case 2:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "ASR");
			break;
		default:
			snprintf(thumb_instruction, MAX_LEN, "%s", "ERROR: Format 1: Unknown operation");
			return thumb_instruction;
	}

	// isolate rd bits 0-2
	hold = isolate_bits(opcode, 0, 2);
		//(opcode & 0x7);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d, ", hold);

	// isolate rs bits 3-5
	hold = isolate_bits(opcode, 3, 5);
		// ((opcode & 0x38) >> 3);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d, ", hold);

	// isolate offset bits 6-10
	hold = isolate_bits(opcode, 6, 10);
		// ((opcode & 0x7C0) >> 6);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "#%d", hold);

	return thumb_instruction;
}

static char *disassemble_format2(uint16_t opcode)
{
	uint16_t hold, is_immediate;
	int i = 0;
	
	memset(thumb_instruction, 0, MAX_LEN);
	// isolate op bit 9 - ADD or SUB
	hold = isolate_bits(opcode, 9, 9);
		// ((opcode & 0x0200) >> 9);
	if (hold == 0) {
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "ADD");
	} else {
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "SUB");
	}
	// isolate rd bits 0-2
	hold = isolate_bits(opcode, 0, 2);
		// (opcode & 0x7);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d, ", hold);
	// isolate rs bits 3-5
	hold = isolate_bits(opcode, 3, 5);
		// ((opcode & 0x38) >> 3);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d, ", hold);
	// isolate immediate flag bit 10
	is_immediate = isolate_bits(opcode, 10, 10);
		// ((opcode & 0x0400) >> 10);
	// isolate rn/offset bits 6-8
	hold = isolate_bits(opcode, 6, 8);
		// ((opcode & 0x1C0) >> 6);
	if (is_immediate) {
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "#%d", hold);
	} else {
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d", hold);
	}

	return thumb_instruction;
}
static char *disassemble_format3(uint16_t opcode)
{
	uint16_t hold;
	int i = 0;
	
	memset(thumb_instruction, 0, MAX_LEN);

	// isolate op bits 11-12
	hold = isolate_bits(opcode, 11, 12);
		// ((opcode & 0x1800) >> 11);
	switch (hold) {
		case 0:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "MOV");
			break;
		case 1:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "CMP");
			break;
		case 2:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "ADD");
			break;
		case 3:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "SUB");
			break;
		default:
			snprintf(thumb_instruction, MAX_LEN, "%s", "ERROR: Format 3: Unknown operation");
			return thumb_instruction;
	}
	// isolate rd bits 8-10
	hold = isolate_bits(opcode, 8, 10);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d, ", hold);
	// isolate offset bits 0-7
	hold = isolate_bits(opcode, 0, 7);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "#%d", hold);
	
	return thumb_instruction;
}

static char *disassemble_format4_5_decider(uint16_t opcode)
{
	uint16_t hold;
	
	// isolate bit 10 to see if it is alu or high_reg_branch_exchange instruction
	hold = isolate_bits(opcode, 10, 10);

	if (hold == 1){
		return disassemble_format4(opcode);
	} else {
		return disassemble_format5(opcode);
	}
}

static char *disassemble_format4(uint16_t opcode)
{
	uint16_t hold;
	int i = 0;
	
	memset(thumb_instruction, 0, MAX_LEN);

	// isolate op bits 6-9
	hold = isolate_bits(opcode, 6, 9);
	// switch case on op
	switch (hold) {
		case 0:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "AND");
			break;
		case 1:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "EOR");
			break;
		case 2:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "LSL");
			break;
		case 3:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "LSR");
			break;
		case 4:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "ASR");
			break;
		case 5:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "ADC");
			break;
		case 6:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "SBC");
			break;
		case 7:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "ROR");
			break;
		case 8:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "TST");
			break;
		case 9:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "NEG");
			break;
		case 10:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "CMP");
			break;
		case 11:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "CMN");
			break;
		case 12:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "ORR");
			break;
		case 13:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "MUL");
			break;
		case 14:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "BIC");
			break;
		case 15:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "MVN");
			break;
		default:
			snprintf(thumb_instruction, MAX_LEN, "%s", "ERROR: Format 4: Unknown operation");
			return thumb_instruction;
	}

	// isolate rd bits 0-2
	hold = isolate_bits(opcode, 0, 2);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d, ", hold);
	// isolate rs bits 3-5
	hold = isolate_bits(opcode, 3, 5);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d", hold);

	return thumb_instruction;
}

static char *disassemble_format5(uint16_t opcode)
{
	uint16_t hold;
	int i = 0;
	
	memset(thumb_instruction, 0, MAX_LEN);

	// isolate op bits 8-9
	hold = isolate_bits(opcode, 8, 9);

	// if h1 and h2 bits 6-7 are zero and op is 0, 1 or 2 then invalid instruction 
	if (((hold >= 0) && (hold <= 2)) && (isolate_bits(opcode, 6, 7) == 0)) {
		snprintf(thumb_instruction, MAX_LEN, "%s",
			"ERROR: Format 5: Both regs can't be low for ops ADD, CMP and MOV.");
		return thumb_instruction;	
	}

	// if op is 3 and h1 is 1 then invalid instruction
	if ((hold == 3) && (isolate_bits(opcode, 7, 7) == 1)) {
		snprintf(thumb_instruction, MAX_LEN, "%s",
			"ERROR: Format 5: H1 cannot be 1 for BX instruction.");
		return thumb_instruction;
	}

	switch (hold) {
		case 0:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "ADD");
			break;
		case 1:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "CMP");
			break;
		case 2:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "MOV");
			break;
		case 3:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "BX");
			break;
		default:
			snprintf(thumb_instruction, MAX_LEN, "%s", "ERROR: Format 5: Unknown operation");
			return thumb_instruction;
	}

	// ignore rd if op is BX
	if (hold != 3) {
		// rd bits 0-2
		hold = isolate_bits(opcode, 0, 2);
		// if h1 bit 7 set then add 8 to rd
		if (isolate_bits(opcode, 7, 7) == 1)
			hold += 8;
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d, ", hold);
	}

	// rs bit 3-5
	hold = isolate_bits(opcode, 3, 5);
	// if h2 bit 6 set then add 8 to rs
	if (isolate_bits(opcode, 6, 6) == 1)
		hold += 8;
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d", hold);

	return thumb_instruction;
}

static char *disassemble_format6(uint16_t opcode)
{
	uint16_t hold;
	int i = 0;
	
	memset(thumb_instruction, 0, MAX_LEN);

	i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "LDR");

	// isolate rd bits 8-10
	hold = isolate_bits(opcode, 8, 10);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d, ", hold);

	// isolate imm bits 0-7
	hold = isolate_bits(opcode, 0, 7);
	hold = hold << 2;
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "[PC, #%d]", hold);
	
	return thumb_instruction;
}

static char *disassemble_format7_8_decider(uint16_t opcode)
{
	if (isolate_bits(opcode, 9, 9) == 0)
		return disassemble_format7(opcode);
	else
		return disassemble_format8(opcode);
}

static char *disassemble_format7(uint16_t opcode)
{
	uint16_t hold;
	int i = 0;
	
	memset(thumb_instruction, 0, MAX_LEN);

	// isolate bit 11 to decide whether it's STR(B) or LDR(B)
	hold = isolate_bits(opcode, 11, 11);
	if (hold == 0)
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s", "STR");
	else
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s", "LDR");

	// isolate bit 10. if set then add "B " otherwise " "
	hold = isolate_bits(opcode, 10, 10);
	if (hold == 0)
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s", " ");
	else
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "B");
	// isolate rd bits 0-2
	hold = isolate_bits(opcode, 0, 2);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d, ", hold);

	// isolate rb bits 3-5
	hold = isolate_bits(opcode, 3, 5);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "[R%d, ", hold);

	// isolate ro bits 6-8
	hold = isolate_bits(opcode, 6, 8);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d]", hold);

	return thumb_instruction;
}

static char *disassemble_format8(uint16_t opcode)
{
	uint16_t hold;
	int i = 0;
	
	memset(thumb_instruction, 0, MAX_LEN);

	// isolate op bits 10-11
	hold = isolate_bits(opcode, 10, 11);

	// switch-case over four possibilities
	switch(hold) {
		case 0:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "STRH");
			break;
		case 1:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "LDSB");
			break;
		case 2:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "LDRH");
			break;
		case 3:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "LDSH");
			break;
		default:
			i += snprintf(thumb_instruction, MAX_LEN,
				"ERROR: Format 8: Unknown op: %d", hold);
			return thumb_instruction;
	}

	// extract rd bits 0-2
	hold = isolate_bits(opcode, 0, 2);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d, ", hold);


	// extract rb bits 3-5
	hold = isolate_bits(opcode, 3, 5);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "[R%d, ", hold);

	// extract ro bits 6-8
	hold = isolate_bits(opcode, 6, 8);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d]", hold);

	return thumb_instruction;
}

static char *disassemble_format9(uint16_t opcode)
{
	uint16_t hold;
	int i = 0;

	memset(thumb_instruction, 0, MAX_LEN);

	// isolate op bits 11-12
	hold = isolate_bits(opcode, 11, 12);
	switch (hold) {
		case 0:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "STR");
			break;
		case 1:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "LDR");
			break;
		case 2:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "STRB");
			break;
		case 3:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "LDRB");
			break;
		default:
			i += snprintf(thumb_instruction, MAX_LEN,
				"ERROR: Format 9: Unknown op: %d", hold);
			return thumb_instruction;
	}

	// isolate rd bits 0-2
	hold = isolate_bits(opcode, 0, 2);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d, ", hold);
	// isolate rb bits 3-5
	hold = isolate_bits(opcode, 3, 5);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "[R%d, ", hold);
	// isolate offset 6-10
	hold = isolate_bits(opcode, 6, 10);
	// if it's a word access then last two bits, both zero, are implied
	if (isolate_bits(opcode, 12, 12) == 0)
		hold = hold << 2;
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "#%d]", hold);

	return thumb_instruction;
}

static char *disassemble_format10(uint16_t opcode)
{
	uint16_t hold;
	int i = 0;

	memset(thumb_instruction, 0, MAX_LEN);

	// isolate op bit 11
	hold = isolate_bits(opcode, 11, 11);
	if (hold == 0)
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "STRH");
	else
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "LDRH");
	
	// isolate rd bits 0-2
	hold = isolate_bits(opcode, 0, 2);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d, ", hold);

	// isolate rb bits 3-5
	hold = isolate_bits(opcode, 3, 5);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "[R%d, ", hold);

	// isolate offset5 bits 6-10
	hold = isolate_bits(opcode, 6, 10);
	hold = hold << 1;
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "#%d]", hold);

	return thumb_instruction;
}

static char *disassemble_format11(uint16_t opcode)
{
	uint16_t hold;
	int i = 0;

	memset(thumb_instruction, 0, MAX_LEN);

	// isolate op bit 11
	hold = isolate_bits(opcode, 11, 11);
	if (hold == 0)
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "STR");
	else
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "LDR");

	// isolate rd bits 8-10
	hold = isolate_bits(opcode, 8, 10);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d, ", hold);	

	// isolate word8 bits 0-7
	hold = isolate_bits(opcode, 0, 7);
	// left shift it by 2
	hold = hold << 2;
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "[SP, #%d]", hold);

	return thumb_instruction;
}

static char *disassemble_format12(uint16_t opcode)
{
	uint16_t hold;
	int i = 0;

	memset(thumb_instruction, 0, MAX_LEN);

	i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "ADD");

	// isolate rd bits 8-10
	hold = isolate_bits(opcode, 8, 10);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d, ", hold);

	// isolate PC/SP bit 11
	hold = isolate_bits(opcode, 11, 11);
	if (hold == 0)
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s, ", "PC");
	else
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s, ", "SP");

	// isolate word8 bits 0-7
	hold = isolate_bits(opcode, 0, 7);
	// left shift it by two bits
	hold = hold << 2;
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "#%d", hold);

	return thumb_instruction;
}

static char *disassemble_format13_14_decider(uint16_t opcode)
{
	if (isolate_bits(opcode, 10, 10) == 0)
		return disassemble_format13(opcode);
	else
		return disassemble_format14(opcode);
}

static char *disassemble_format13(uint16_t opcode)
{
	uint16_t hold;
	int i = 0;

	memset(thumb_instruction, 0, MAX_LEN);

	i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s", "ADD SP, #");

	// isolate sword7 bits 0-6
	hold = isolate_bits(opcode, 0, 6);
	// left shift by 2
	hold = hold << 2;

	// isolate sign bit 7
	if (isolate_bits(opcode, 7, 7) == 0)
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%d", hold);
	else
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "-%d", hold);

	return thumb_instruction;
}

static char *disassemble_format14(uint16_t opcode)
{
	uint16_t hold;
	int i = 0;
	uint8_t reg_mask_shift = 0;

	memset(thumb_instruction, 0, MAX_LEN);

	// isloate L bit 11
	if (isolate_bits(opcode, 11, 11) == 0)
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "PUSH");
	else
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "POP");

	i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s", "{");
	// isolate Rlist bits 0-7
	hold = isolate_bits(opcode, 0, 7);
	for (reg_mask_shift = 0; reg_mask_shift < 8; reg_mask_shift++) {
		if (hold & (1 << reg_mask_shift))
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d, ", reg_mask_shift);
	}

	// isolate R bit 8
	if (isolate_bits(opcode, 8, 8)) 
		if (isolate_bits(opcode, 11, 11) == 0)
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s}", "LR");
		else
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s}", "PC");	
	else
		i += snprintf(thumb_instruction + i - 2, MAX_LEN - i, "%s", "}");

	return thumb_instruction;
}

static char *disassemble_format15(uint16_t opcode)
{
	uint16_t hold;
	int i = 0;
	uint8_t reg_mask_shift = 0;

	memset(thumb_instruction, 0, MAX_LEN);

	// isolate load/store bit 11
	if (isolate_bits(opcode, 11, 11) == 0)
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "STMIA");
	else
		i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "LDMIA");

	// isolate rb bits 8-10
	hold = isolate_bits(opcode, 8, 10);
	i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d!, ", hold);

	i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s", "{");
	// isolate Rlist bits 0-7
	hold = isolate_bits(opcode, 0, 7);
	for (reg_mask_shift = 0; reg_mask_shift < 8; reg_mask_shift++) {
		if (hold & (1 << reg_mask_shift))
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "R%d, ", reg_mask_shift);
	}

	i += snprintf(thumb_instruction + i - 2, MAX_LEN - i, "%s", "}");

	return thumb_instruction;
}

static char *disassemble_format16_17_decider(uint16_t opcode)
{
	// if bits 8-11 are all 1s that's format 17 
	if (isolate_bits(opcode, 8, 11) == 0xF)
		return disassemble_format17(opcode);
	else
		return disassemble_format16(opcode);
}

static char *disassemble_format16(uint16_t opcode)
{
	uint16_t hold;
	int i = 0;

	memset(thumb_instruction, 0, MAX_LEN);

	// isolate cond bits 8-11
	hold = isolate_bits(opcode, 8, 11);
	switch(hold) {
		case 0:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "BEQ");
			break;
		case 1:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "BNE");
			break;
		case 2:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "BCS");
			break;
		case 3:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "BCC");
			break;
		case 4:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "BMI");
			break;
		case 5:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "BPL");
			break;
		case 6:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "BVS");
			break;
		case 7:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "BVC");
			break;
		case 8:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "BHI");
			break;
		case 9:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "BLS");
			break;
		case 10:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "BGE");
			break;
		case 11:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "BLT");
			break;
		case 12:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "BGT");
			break;
		case 13:
			i += snprintf(thumb_instruction + i, MAX_LEN - i, "%s ", "BLE");
			break;
		default:
			i += snprintf(thumb_instruction, MAX_LEN,
				"ERROR: Format 16: Unknown op: %d", hold);
			return thumb_instruction;
	}

	// islolate offset bits 0-7
	hold = isolate_bits(opcode, 0, 7);
	hold = hold << 1;
	// if most significant bit 7 is set then sign-extend as this is two's complement
	if (hold & 0x100)
		hold = hold | 0xFF00;

	i += snprintf(thumb_instruction + i, MAX_LEN - i,
		"label ;label = PC + (%d). Note that PC is curr instruction + 4 due to instruction prefetch.",
		(int16_t) hold);

	return thumb_instruction;
}

static char *disassemble_format17(uint16_t opcode)
{
	uint16_t hold;
	
	memset(thumb_instruction, 0, MAX_LEN);

	// isolate bits interrupt number bits 0-7
	hold = isolate_bits(opcode, 0, 7);
	snprintf(thumb_instruction, MAX_LEN, "SWI %d", hold);

	return thumb_instruction;
}

static char *disassemble_format18(uint16_t opcode)
{
	uint16_t hold;
	
	memset(thumb_instruction, 0, MAX_LEN);

	// isolate offset bits 0-10
	hold = isolate_bits(opcode, 0, 10);
	hold = hold << 1;
	// if most significant bit 11 is set then sign-extend as this is two's complement
	if (hold & 0x800)
		hold = hold | 0xF000;

	snprintf(thumb_instruction, MAX_LEN,
		"B label ;label = PC + (%d) - Note that PC = curr instruction + 4 due to instruction prefetch",
		(int16_t)hold);

	return thumb_instruction;
}

static char *disassemble_format19(uint16_t opcode)
{
	uint16_t hold;
	int i = 0;
	
	memset(thumb_instruction, 0, MAX_LEN);

	i += snprintf(thumb_instruction + i, MAX_LEN - i,
		"BL label ; where label = PC + 23-bit num in two's complement");

	hold = isolate_bits(opcode, 0, 10);
	if (isolate_bits(opcode, 11, 11) == 0)
		i += snprintf(thumb_instruction + i, MAX_LEN - i,
			" - 11 high bits of the number are 0x%x", hold);
	else
		i += snprintf(thumb_instruction + i, MAX_LEN - i,
			" - 12 low bits of the number are 0x%x", (hold << 1));

	return thumb_instruction;
}

/***** helper functions *****/
static uint16_t isolate_bits(uint16_t src, uint8_t start_bit, uint8_t end_bit)
{
	src = src << (15 - end_bit);
	src = src >> (start_bit + 15 - end_bit);

	return src;
}