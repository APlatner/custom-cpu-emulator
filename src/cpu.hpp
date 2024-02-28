#pragma once

#include "../assembly/instruction.hpp"

class CPU {
public:
	bool run(const char *file_name);
	~CPU();

	void draw();
	void fetch();
	void interpret();
	void restart();

	bool get_running() { return running; }

private:
	void set_reg(int reg);

	void bitwise_reg();
	void arithmetic_reg();
	void shift_reg();

	void bitwise_imm();
	void arithmetic_imm();
	void shift_imm();

	void jump_imm();
	void ram_op_imm();
	void ram_op_reg();
	void interupt_reg();

	void move_imm();
	void move_reg();

	bool running;

	unsigned char ram[1024];
	char terminal[64];
	int terminal_index;
	short registers[REG_MAX];
};
