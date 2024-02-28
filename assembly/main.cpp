#include "instruction.hpp"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <map>
#include <string>
#include <vector>

static short write_pointer = 2;
static int mode = 0;
static FILE *bin_file;
std::map<std::string, short> labels;

enum AssemblerMode {
	DATA_MODE = 1,
	CODE_MODE,
};

void write_instruction(short instruction) {
	write_pointer += 2;
	fwrite(&instruction, 2, 1, bin_file);
	printf("instruction: %04hx\n", instruction);
}

int get_register(int &reg) {
	char *token = strtok(NULL, " \n");
	if (token == NULL) {
		printf("failed to get register!\n");
		return REGISTER_ERROR;
	}
	if (strncmp(token, "ra", 2) == 0) {
		reg = REG_A;
	} else if (strncmp(token, "rb", 2) == 0) {
		reg = REG_B;
	} else if (strncmp(token, "rc", 2) == 0) {
		reg = REG_C;
	}
	return NO_ERROR;
}

int get_immediate(int &imm, bool &is_label) {
	char *token = strtok(NULL, " \n");
	for (auto label : labels) {
		if (token == label.first) {
			imm = label.second;
			is_label = true;
			return NO_ERROR;
		}
	}
	imm = std::stoi(token);
	is_label = false;

	return NO_ERROR;
}

void ld_st_code(int code, char *token) {
	bool immediate_type = false;
	if (token[3] == 'i') {
		code += 4;
		immediate_type = true;
	}
	short instruction = code;
	int reg = 0;
	if (get_register(reg)) {
		printf("failed to get reg!\n");
		return;
	}
	instruction |= (reg << 5);
	if (immediate_type) {
		int imm;
		bool is_label;
		get_immediate(imm, is_label);
		imm -= write_pointer;
		instruction |= imm * 256;
	} else {
		if (get_register(reg)) {
			printf("failed to get reg!\n");
			return;
		}
		instruction |= (reg << 8);
	}

	write_instruction(instruction);
}

void reg_reg(short code) {
	short instruction = code;
	int reg = 0;
	if (get_register(reg)) {
		printf("failed to get reg!\n");
		return;
	}
	instruction |= (reg << 5);
	if (get_register(reg)) {
		printf("failed to get reg!\n");
		return;
	}
	instruction |= (reg << 8);
	write_instruction(instruction);
}

void load_jmp(int func) {
	short instruction = JUMP;
	instruction |= func << 5;
	int imm;
	bool is_label;
	get_immediate(imm, is_label);
	instruction |= (imm - write_pointer - 2) * 256;
	write_instruction(instruction);
}

void declare_byte() {
	char *token = strtok(NULL, " ");
	std::string bytes;
	if (token[0] == '\"') {
		bytes = &token[1];
		bytes += ' ';
		bytes += strtok(NULL, "\"");
	}
	token = strtok(NULL, " ,\n");
	char c = (char)strtol(token, &token, 10);
	bytes += c;
	fwrite(bytes.c_str(), bytes.length(), 1, bin_file);
	write_pointer += bytes.length();
}

void declare_word() {
	char *token = strtok(NULL, " ,\n");
	short number = strtol(token, &token, 10);
	fwrite(&number, sizeof(number), 1, bin_file);
	write_pointer += sizeof(number);
}

void load_data() {
	char *token = strtok(NULL, " ,\n");
	if (token != NULL) {
		if (strncmp(token, "db", 2) == 0) {
			declare_byte();
		} else if (strncmp(token, "dw", 2) == 0) {
			declare_word();
		}
	}
}

int arith_code(int code, char *token) {
	int index;
	if (code == OR) {
		index = 2;
	} else {
		index = 3;
	}
	if (token[index] == 'i') {
		code += ANDI;
		index++;
	}
	int func = 0;
	if (token[index] == 's') {
		func |= 1;
		index++;
	}
	if (token[index] == 'u') {
		func |= 2;
		index++;
	}
	if (token[index] == 'o') {
		func |= 4;
		index++;
	}
	short instruction = code;
	int reg = 0;
	int result = get_register(reg);
	if (result) {
		return result;
	}
	instruction |= (reg << 5);
	if (code > 9) {
		int imm = 0;
		bool is_label;
		result = get_immediate(imm, is_label);
		if (result) {
			return result;
		}
		instruction |= (func << 8);
		instruction |= (short)(imm << 11);
	} else {
		result = get_register(reg);
		if (result) {
			return result;
		}
		instruction |= (func << 8);
		instruction |= (reg << 11);
	}

	write_instruction(instruction);

	return 0;
}

void interupt(int func) {
	short instruction = INT;
	int reg;
	int result = get_register(reg);
	if (result) {
		return;
	}
	instruction |= (reg << 5);
	instruction |= (func << 8);
	write_instruction(instruction);
}

bool assemble(const std::string &asm_file_name) {
	FILE *asm_file = fopen(asm_file_name.c_str(), "r");
	if (asm_file == NULL) {
		return false;
	}

	bin_file = fopen("raw_binary_file", "wb");
	if (bin_file == NULL) {
		return false;
	}
	char buffer[256];
	short jmp_insert = 0;
	fwrite(&jmp_insert, 2, 1, bin_file);
	while (!feof(asm_file) && strncmp(buffer, "hlt", 3) != 0) {
		fgets(buffer, 255, asm_file);
		// printf("%s\n", buffer);
		char *token = strtok(buffer, " :\n");
		if (token == NULL) {
			continue;
		}

		if (strncmp("and", token, 3) == 0) {
			arith_code(AND, token);
		} else if (strncmp("or", token, 2) == 0) {
			arith_code(OR, token);
		} else if (strncmp("xor", token, 3) == 0) {
			arith_code(XOR, token);
		} else if (strncmp("add", token, 3) == 0) {
			arith_code(ADD, token);
		} else if (strncmp("sub", token, 3) == 0) {
			arith_code(SUB, token);
		} else if (strncmp("muli", token, 4) == 0) {
			arith_code(ADD, token);
		} else if (strncmp("div", token, 3) == 0) {
			arith_code(DIV, token);
		} else if (strncmp("mod", token, 3) == 0) {
			arith_code(MOD, token);
		} else if (strncmp("not", token, 3) == 0) {
			// not is a pseudo-instruction represented by xori dest,
			// src, -1
			short instruction = XORI;
			int reg;
			if (get_register(reg)) {
				printf("failed to get reg!\n");
				return false;
			}
			instruction |= (reg << 5);
			instruction |= 0b1111111100000000;
			write_instruction(instruction);
		} else if (strncmp("nop", token, 3) == 0) {
			// nop is a pseudo-instruction represented by addi ra,
			// ra, 0
			short instruction = ADDI;
			write_instruction(instruction);
		} else if (strncmp("movi", token, 4) == 0) {
			short instruction = MOVI;
			int reg;
			if (get_register(reg)) {
				printf("failed to get reg!\n");
				return false;
			}
			instruction |= (reg << 5);
			int imm;
			bool is_label;
			get_immediate(imm, is_label);
			if (!is_label) {
				instruction |= imm * 512;
			} else {
				instruction |= (1 << 8);
				instruction |= (imm - write_pointer - 2) * 512;
			}

			write_instruction(instruction);
		} else if (strncmp("mov", token, 3) == 0) {
			short instruction = MOV;
			int reg;
			if (get_register(reg)) {
				printf("failed to get reg!\n");
				return false;
			}
			instruction |= (reg << 5);
			if (get_register(reg)) {
				printf("failed to get reg!\n");
				return false;
			}
			instruction |= reg << 8;
			write_instruction(instruction);
		} else if (strncmp("jmp", token, 3) == 0) {
			load_jmp(JMP);
		} else if (strncmp("jal", token, 3) == 0) {
			load_jmp(JAL);
		} else if (strncmp("jeq", token, 3) == 0) {
			load_jmp(JEQ);
		} else if (strncmp("jne", token, 3) == 0) {
			load_jmp(JNE);
		} else if (strncmp("jlt", token, 3) == 0) {
			load_jmp(JLT);
		} else if (strncmp("jle", token, 3) == 0) {
			load_jmp(JLE);
		} else if (strncmp("jgt", token, 3) == 0) {
			load_jmp(JGT);
		} else if (strncmp("jge", token, 3) == 0) {
			load_jmp(JGE);
		} else if (strncmp("stw", token, 3) == 0) {
			ld_st_code(STW, token);
		} else if (strncmp("stb", token, 3) == 0) {
			ld_st_code(STB, token);
		} else if (strncmp("ldw", token, 3) == 0) {
			ld_st_code(LDW, token);
		} else if (strncmp("ldb", token, 3) == 0) {
			ld_st_code(LDB, token);
		} else if (strncmp("out", token, 3) == 0) {
			interupt(OUT);
		} else if (strncmp("in", token, 2) == 0) {
			interupt(IN);
		} else if (strncmp("hlt", token, 3) == 0) {
			write_instruction(HLT);
		} else if (strncmp(".data", token, 5) == 0) {
			printf("storing data\n");
			mode = DATA_MODE;
		} else if (strncmp(".code", token, 5) == 0) {
			printf("assembling code\n");
			mode = CODE_MODE;
		} else {
			printf("%s: %hi\n", token, write_pointer);
			labels.emplace(std::pair(token, write_pointer));
			if (mode == DATA_MODE) {
				load_data();
			}
		}
	}

	// short instruction = JUMP;
	// instruction |= func << 5;
	// int imm;
	// get_immediate(imm);
	// instruction |= imm * 256;
	// write_instruction(instruction);

	fseek(bin_file, 0, SEEK_SET);
	short instruction = 0;
	std::string label = "_start";
	for (auto l : labels) {
		if (label == l.first) {
			instruction = JUMP;
			instruction |= JMP << 5;
			instruction |= (l.second - 2) * 256;
		}
	}
	write_instruction(instruction);

	fclose(asm_file);
	fclose(bin_file);

	return true;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("must provide assembly file!\n");
		return 1;
	}

	return !assemble(argv[1]);
}