#include "cpu.hpp"

#include <cstdio>
#include <cstring>

bool CPU::run(const char *file_name) {
	FILE *bin_file = fopen(file_name, "rb");
	if (bin_file == nullptr) {
		return false;
	}
	fseek(bin_file, 0, SEEK_END);
	long file_size = ftell(bin_file);
	fseek(bin_file, 0, SEEK_SET);
	fread(ram, file_size, 1, bin_file);
	fclose(bin_file);

	running = true;

	return true;
}

CPU::~CPU() {
	FILE *ram_file = fopen("ram.txt", "w");

	for (int i = 0; i < 1024 / 8; i++) {
		for (int j = 0; j < 8; j++) {
			fprintf(ram_file, "%02hhx ", ram[j + i * 8]);
		}
		fprintf(ram_file, "\n");
	}

	fclose(ram_file);
}

void CPU::draw() {
	printf("Registers                                       "
	       " RAM\n");
	printf("RA: 0x%04hx\tProgram Counter      0x%04hx\t "
	       "0x%02hhx\n",
	    registers[REG_A], registers[PC], ram[registers[RAM_ADDRESS]]);
	printf("RB: 0x%04hx\tInstruction Register 0x%04hx\t "
	       "0x%02hhx\n",
	    registers[REG_B], registers[INSTR_REG],
	    ram[registers[RAM_ADDRESS] + 1]);
	printf("RC: 0x%04hx\tCPU Flags Register   0x%04hx\n\n",
	    registers[REG_C], registers[CPU_FLAGS]);
	printf("%s", terminal);
}

void CPU::fetch() {
	if (!running) {
		return;
	}
	registers[INSTR_REG] = ram[registers[PC]];
	registers[PC] += 1;
	registers[INSTR_REG] |= ram[registers[PC]] << 8;
	registers[PC] += 1;
}

void CPU::interpret() {
	if (!running) {
		return;
	}
	if (registers[INSTR_REG] == 0) {
		running = false;
	}
	int code = registers[INSTR_REG] & 0b0000000000011111;
	switch (code) {
	case AND:
	case OR:
	case XOR:
		bitwise_reg();
		break;
	case LS:
		shift_reg();
		break;
	case ADD:
	case SUB:
	case MUL:
	case DIV:
	case MOD:
		arithmetic_reg();
		break;
	case ANDI:
	case ORI:
	case XORI:
		bitwise_imm();
		break;
	case LSI:
		shift_imm();
		break;
	case ADDI:
	case SUBI:
	case MULI:
	case DIVI:
	case MODI:
		arithmetic_imm();
		break;
	case JUMP:
		jump_imm();
		break;
	case STW:
	case STB:
	case LDW:
	case LDB:
		ram_op_reg();
		break;
	case STWI:
	case STBI:
	case LDWI:
	case LDBI:
		ram_op_imm();
		break;
	case MOV:
		move_reg();
		break;
	case MOVI:
		move_imm();
		break;
	case INT:
		interupt_reg();
		break;
	case HLT:
		running = false;
		break;
	}
}

void CPU::restart() {
	memset(registers, 0, sizeof(registers));
	memset(terminal, 0, sizeof(terminal));
	terminal_index = 0;
	running = true;
}

void CPU::set_reg(int reg) {
	registers[CPU_FLAGS] = ((registers[reg] == 0) << ZERO_FLAG) |
	                       (~(1 << ZERO_FLAG) & registers[CPU_FLAGS]);
	registers[CPU_FLAGS] = ((registers[reg] < 0) << NEGATIVE_FLAG) |
	                       (~(1 << NEGATIVE_FLAG) & registers[CPU_FLAGS]);
}

void CPU::bitwise_reg() {
	int code = registers[INSTR_REG] & 0b0000000000011111;
	int dest = (registers[INSTR_REG] & 0b0000000011100000) >> 5;
	int func = (registers[INSTR_REG] & 0b0000011100000000) >> 8;
	int src = (registers[INSTR_REG] & 0b0011100000000000) >> 11;

	switch (code) {
	case AND:
		registers[dest] &= registers[src];
		break;
	case OR:
		registers[dest] |= registers[src];
		break;
	case XOR:
		registers[dest] ^= registers[src];
		break;
	}
	if ((func & (1 << SET_FLAGS)) != 0) {
		set_reg(dest);
	}
}

void CPU::arithmetic_reg() {
	int code = registers[INSTR_REG] & 0b0000000000011111;
	int dest = (registers[INSTR_REG] & 0b0000000011100000) >> 5;
	int func = (registers[INSTR_REG] & 0b0000011100000000) >> 8;
	int src = (registers[INSTR_REG] & 0b0011100000000000) >> 11;
	int carry = (func & (1 << OVERFLOW)) &&
	            (registers[CPU_FLAGS] & (1 << OVERFLOW_FLAG));
	switch (code) {
	case ADD:
		registers[dest] += registers[src] + carry;
		break;
	case SUB:
		registers[dest] -= registers[src] - carry;
		break;
	case MUL:
		registers[dest] *= registers[src];
		break;
	case DIV:
		registers[dest] /= registers[src];
		break;
	case MOD:
		registers[dest] %= registers[src];
		break;
	}
	if ((func & (1 << SET_FLAGS)) != 0) {
		set_reg(dest);
	}
}

void CPU::shift_reg() {
	int code = registers[INSTR_REG] & 0b0000000000011111;
	int dest = (registers[INSTR_REG] & 0b0000000011100000) >> 5;
	int func = (registers[INSTR_REG] & 0b0000011100000000) >> 8;
	int src = (registers[INSTR_REG] & 0b0011100000000000) >> 11;
	if ((func & (1 << UNSIGNED)) != 0) {
		// logical
		unsigned short dest_reg = registers[dest];
		switch (code) {
		case LS: {
			if (registers[src] < 0) {
				registers[dest] =
				    dest_reg >> (registers[dest] * -1);
			} else {
				registers[dest] = dest_reg << registers[dest];
			}
		} break;
		}
	} else {
		// arithmetic
		switch (code) {
		case LS: {
			if (registers[src] < 0) {
				registers[dest] =
				    registers[dest] >> (registers[src] * -1);
			} else {
				registers[dest] = registers[dest]
				                  << registers[src];
			}
		} break;
		}
		if ((func & (1 << SET_FLAGS)) != 0) {
			set_reg(dest);
		}
	}
}

void CPU::bitwise_imm() {
	int code = registers[INSTR_REG] & 0b0000000000011111;
	int dest = (registers[INSTR_REG] & 0b0000000011100000) >> 5;
	int func = (registers[INSTR_REG] & 0b0000011100000000) >> 8;
	short imm = (registers[INSTR_REG] & 0b1111100000000000) >> 11;

	switch (code) {
	case ANDI:
		registers[dest] &= imm;
		break;
	case ORI:
		registers[dest] |= imm;
		break;
	case XORI:
		registers[dest] ^= imm;
		break;
	}
	if ((func & (1 << SET_FLAGS)) != 0) {
		set_reg(dest);
	}
}

void CPU::arithmetic_imm() {
	int code = registers[INSTR_REG] & 0b0000000000011111;
	int dest = (registers[INSTR_REG] & 0b0000000011100000) >> 5;
	int func = (registers[INSTR_REG] & 0b0000011100000000) >> 8;
	short imm = (registers[INSTR_REG] & 0b1111100000000000) >> 11;
	int carry = (func & (1 << OVERFLOW)) &&
	            (registers[CPU_FLAGS] & (1 << OVERFLOW_FLAG));
	switch (code) {
	case ADDI:
		registers[dest] += imm + carry;
		break;
	case SUBI:
		registers[dest] -= imm - carry;
		break;
	case MULI:
		registers[dest] *= imm;
		break;
	case DIVI:
		registers[dest] /= imm;
		break;
	case MODI:
		registers[dest] %= imm;
		break;
	}
	if ((func & (1 << SET_FLAGS)) != 0) {
		set_reg(dest);
	}
}

void CPU::shift_imm() {
	int code = registers[INSTR_REG] & 0b0000000000011111;
	int dest = (registers[INSTR_REG] & 0b0000000011100000) >> 5;
	int func = (registers[INSTR_REG] & 0b0000011100000000) >> 8;
	short imm = (registers[INSTR_REG] & 0b1111100000000000) >> 11;
	if ((func & (1 << UNSIGNED)) != 0) {
		// logical
		unsigned short dest_reg = registers[dest];
		switch (code) {
		case LSI: {
			if (imm < 0) {
				registers[dest] = dest_reg >> (imm * -1);
			} else {
				registers[dest] = dest_reg << imm;
			}
		} break;
		}
	} else {
		// arithmetic
		switch (code) {
		case LSI: {
			if (imm < 0) {
				registers[dest] = registers[dest] >> (imm * -1);
			} else {
				registers[dest] = registers[dest] << imm;
			}
		} break;
		}
	}
	if ((func & (1 << SET_FLAGS)) != 0) {
		set_reg(dest);
	}
}

void CPU::jump_imm() {
	int func = (registers[INSTR_REG] & 0b0000000011100000) >> 5;
	short imm = (short)(registers[INSTR_REG] & 0b1111111100000000) >> 8;
	switch (func) {
	case JMP:
		registers[PC] += imm;
		break;
	case JAL: {
		registers[LR] = registers[PC];
		registers[PC] += imm;
	} break;
	case JEQ: {
		if ((registers[CPU_FLAGS] & (1 << ZERO_FLAG)) != 0) {
			registers[PC] += imm;
		}
	} break;
	case JNE: {
		if ((registers[CPU_FLAGS] & (1 << ZERO_FLAG)) == 0) {
			registers[PC] += imm;
		}
	} break;
	case JLT: {
		if ((registers[CPU_FLAGS] & (1 << NEGATIVE_FLAG)) != 0) {
			registers[PC] += imm;
		}
	} break;
	case JLE: {
		if (((registers[CPU_FLAGS] & (1 << ZERO_FLAG)) != 0) ||
		    ((registers[CPU_FLAGS] & (1 << NEGATIVE_FLAG)) != 0)) {
			registers[PC] += imm;
		}
	} break;
		break;
	case JGT: {
		if (((registers[CPU_FLAGS] & (1 << ZERO_FLAG)) == 0) &&
		    ((registers[CPU_FLAGS] & (1 << NEGATIVE_FLAG)) == 0)) {
			registers[PC] += imm;
		}
	} break;
	case JGE: {
		if ((registers[CPU_FLAGS] & (1 << NEGATIVE_FLAG)) == 0) {
			registers[PC] += imm;
		}
	} break;
	}
}

void CPU::ram_op_reg() {
	int code = registers[INSTR_REG] & 0b0000000000011111;
	int reg = (registers[INSTR_REG] & 0b0000000011100000) >> 5;
	int addr = (registers[INSTR_REG] & 0b0000011100000000) >> 8;
	switch (code) {
	case STW: {
		registers[RAM_ADDRESS] = registers[addr];
		ram[registers[RAM_ADDRESS]] =
		    registers[reg] & 0b0000000011111111;
		ram[registers[RAM_ADDRESS] + 1] =
		    (registers[reg] & 0b1111111100000000) >> 8;
	} break;
	case STB: {
		registers[RAM_ADDRESS] = registers[addr];
		ram[registers[RAM_ADDRESS]] =
		    registers[reg] & 0b0000000011111111;
	} break;
	case LDW: {
		registers[RAM_ADDRESS] = registers[addr];
		registers[reg] = ram[registers[RAM_ADDRESS] + 1] << 8;
		registers[reg] |= ram[registers[RAM_ADDRESS]];
	} break;
	case LDB: {
		registers[RAM_ADDRESS] = registers[addr];
		registers[reg] = ram[registers[RAM_ADDRESS]];
	} break;
	}
}

void CPU::ram_op_imm() {
	int code = registers[INSTR_REG] & 0b0000000000011111;
	int reg = (registers[INSTR_REG] & 0b0000000011100000) >> 5;
	short imm = (registers[INSTR_REG] & 0b1111111100000000) >> 8;
	switch (code) {
	case STWI: {
		registers[RAM_ADDRESS] = registers[PC] + imm;
		ram[registers[RAM_ADDRESS]] =
		    registers[reg] & 0b0000000011111111;
		ram[registers[RAM_ADDRESS] + 1] =
		    (registers[reg] & 0b1111111100000000) >> 8;
	} break;
	case STBI: {
		registers[RAM_ADDRESS] = registers[PC] + imm;
		ram[registers[RAM_ADDRESS]] =
		    registers[reg] & 0b0000000011111111;
	} break;
	case LDWI: {
		registers[RAM_ADDRESS] = registers[PC] + imm;
		registers[reg] = ram[registers[RAM_ADDRESS] + 1] << 8;
		registers[reg] |= ram[registers[RAM_ADDRESS]];
	} break;
	case LDBI: {
		registers[RAM_ADDRESS] = registers[PC] + imm;
		registers[reg] = ram[registers[RAM_ADDRESS]];
	} break;
	}
}

void CPU::interupt_reg() {
	int reg = (registers[INSTR_REG] & 0b0000000011100000) >> 5;
	unsigned short func = (registers[INSTR_REG] & 0b1111111100000000) >> 8;
	switch (func) {
	case OUT: {
		terminal[terminal_index] = (char)registers[reg];
		terminal_index++;
	} break;
	case IN:
		terminal_index--;
		if (terminal[terminal_index] != 0 && terminal_index >= 0) {
			registers[reg] = terminal[terminal_index];
		}
		break;
	case RESET: {
		memset(terminal, 0, sizeof(terminal));
		terminal_index = 0;
	} break;
	}
}

void CPU::move_imm() {
	int dest = (registers[INSTR_REG] & 0b0000000011100000) >> 5;
	int func = (registers[INSTR_REG] & 0b0000000100000000) >> 8;
	short imm = (short)(registers[INSTR_REG] & 0b1111111000000000) >> 9;
	printf("mov: %hx\n", imm);
	if (func != 0) {
		registers[dest] = registers[PC] + imm;
	} else {
		registers[dest] = imm;
	}
}

void CPU::move_reg() {
	int dest = (registers[INSTR_REG] & 0b0000000011100000) >> 5;
	int src = (registers[INSTR_REG] & 0b0000011100000000) >> 8;
	registers[dest] = registers[src];
}
