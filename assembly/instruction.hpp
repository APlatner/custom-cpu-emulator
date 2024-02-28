#pragma once

enum ErrorCode {
	NO_ERROR = 0,
	REGISTER_ERROR,
	LABEL_ERROR,
	IMMEDIATE_ERROR,
};

enum RegisterName {
	REG_A = 0,
	REG_B,
	REG_C,
	SP,
	LR,
	OUTPUT,

	PC,
	INSTR_REG,
	RAM_ADDRESS,
	CPU_FLAGS,

	REG_MAX,
};

enum CpuFlag {
	ZERO_FLAG = 0,
	NEGATIVE_FLAG,
	OVERFLOW_FLAG,
};

enum InstructionCode {
	// {:2} {src:3} {func:3} {dest:3} {code:5}
	// func: {set:1} {unsigned:2} {use overflow:3}
	AND, // 0b--000 000 000 00000
	OR,  // 0b--000 000 000 00001
	XOR, // 0b--000 000 000 00010
	// {:2} {src:3} {func:3} {dest:3} {code:5}
	// func: {set:1} {arithmetic:2} {use overflow:3}
	LS, // 0b--000 000 000 00011
	// {:2} {src:3} {func:3} {dest:3} {code:5}
	// func: {set:1} {unsigned:2} {use overflow:3}
	ADD, // 0b--000 000 000 00101
	SUB, // 0b--000 000 000 00110
	MUL, // 0b--000 000 000 00111
	DIV, // 0b--000 000 000 01000
	MOD, // 0b--000 000 000 01001
	// {imm:5s} {func:3} {dest:3} {code:5}
	// func: {set:1} {unsigned:2} {use overflow:3}
	ANDI, // 0b00000 000 000 01010
	ORI,  // 0b00000 000 000 01011
	XORI, // 0b00000 000 000 01100
	// {imm:5s} {func:3} {dest:3} {code:5}
	// func: {set:1} {arithmetic:2} {use overflow:3}
	LSI, // 0b00000 000 000 01101
	// {imm:5s} {func:3} {dest:3} {code:5}
	// func: {set:1} {unsigned:2} {use overflow:3}
	ADDI, // 0b00000 000 000 01111
	SUBI, // 0b00000 000 000 10000
	MULI, // 0b00000 000 000 10001
	DIVI, // 0b00000 000 000 10010
	MODI, // 0b00000 000 000 10011
	// {imm:8s} {func:3} {code:5}
	JUMP, // 0b00000000 000  10100
	// {imm:5s} {addr:3} {src:3} {code:5}
	STW, // 0b00000 000 000 10101
	STB, // 0b00000 000 000 11000
	// {imm:5s} {addr:3} {dest:3} {code:5}
	LDW, // 0b00000 000 000 11001
	LDB, // 0b00000 000 000 11010
	// {imm:8s} {src:3} {code:5}
	STWI, // 0b00000000 000 10101
	STBI, // 0b00000000 000 11000
	// {imm:8s} {dest:3} {code:5}
	LDWI, // 0b00000000 000 11001
	LDBI, // 0b00000000 000 11010
	// {imm:5s} {src:3} {dest:3} {code:5}
	MOV, // 0b00000 000 000 11011
	// {imm:8s} {func:1} {dest:3} {code:5}
	MOVI, // 0b0000000 0 000 11100
	// {func:8} {src/dest:3} {code:5}
	INT, // 0b0000000 000 11101
	// {imm:11s} {code:5}
	HLT, // 0b00000000000 11110

	MAX_CODE,
	// pseudo-instructions
	// nop = add ra, ra, ra
	// mov {dest}, {src} = add dest, dest, src
	// not {dest}, {src} = xori dest, src, -1
};

enum ArithmeticFuncCode {
	SET_FLAGS,
	UNSIGNED,
	OVERFLOW,
};

enum ConditionJumps {
	JMP, // 0b00000000 000 10001
	JAL, // 0b00000000 001 10001
	JEQ, // 0b00000000 010 10001
	JNE, // 0b00000000 011 10001
	JLT, // 0b00000000 100 10001
	JLE, // 0b00000000 101 10001
	JGT, // 0b00000000 110 10001
	JGE, // 0b00000000 111 10001
};

enum InteruptFunc {
	OUT,
	IN,
	RESET,
};
