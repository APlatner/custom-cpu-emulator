#include "cpu.hpp"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <map>
#include <string>
#include <vector>

#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

constexpr char CLEAR_TERM[] = "\033[2J\033[3J\033[0;0H";
constexpr char SHOW_ALT_BUFFER[] = "\033[?1049h";
constexpr char HIDE_ALT_BUFFER[] = "\033[?1049l";

static termios old_settings, new_settings;
static bool resize_screen = true;
static bool redraw_screen = true;

static CPU cpu;

void sig_handler(int signo) {
	if (signo == SIGWINCH) {
		resize_screen = true;
	}
}

bool start() {
	tcgetattr(STDIN_FILENO, &old_settings);
	new_settings = old_settings;
	new_settings.c_lflag &= ~(ECHO | ICANON);
	new_settings.c_cc[VMIN] = 1;
	new_settings.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);

	write(STDOUT_FILENO, SHOW_ALT_BUFFER, sizeof(SHOW_ALT_BUFFER) - 1);
	// write(STDOUT_FILENO, HIDE_CURSOR, sizeof(HIDE_CURSOR) - 1);
	if (signal(SIGWINCH, sig_handler) == SIG_ERR) {
		printf("Failed to hook into signals!\n");
	}
	fcntl(STDIN_FILENO, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

	return true;
}

void stop() {
	write(STDOUT_FILENO, HIDE_ALT_BUFFER, sizeof(HIDE_ALT_BUFFER) - 1);
	old_settings.c_lflag |= (ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
}

void draw() {
	write(STDOUT_FILENO, CLEAR_TERM, sizeof(CLEAR_TERM) - 1);
	cpu.draw();
	fflush(stdout);
	redraw_screen = false;
}

bool run() {
	long current_time = 0;
	struct timespec now;
	std::string input;
	bool advance = false;
	bool running = true;
	int frame_rate = 16;
	while (running) {
		// if (resize_screen) {
		// 	resize();
		// }
		if (redraw_screen) {
			draw();
		}
		char input_raw[256] = {0};
		fgets(input_raw, 256, stdin);
		input = input_raw;

		switch (input[0]) {
		case 'q':
			running = false;
			break;
		case 'c':
			advance = !advance;
			break;
		case 'r': {
			cpu.restart();
		} break;
		}
		clock_gettime(CLOCK_MONOTONIC, &now);
		if (current_time != now.tv_nsec / (1000000000 / frame_rate) &&
		    advance) {
			// advance = false;
			cpu.fetch();
			cpu.interpret();
			redraw_screen = true;
			current_time = now.tv_nsec / (1000000000 / frame_rate);
		}
	}

	return true;
}

int main() {
	cpu.run("raw_binary_file");

	start();
	run();
	stop();

	return 0;
}