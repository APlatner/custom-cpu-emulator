SRCDIR := src
BINDIR := bin

SRCS := ${shell find ${SRCDIR} -type f -name "*.cpp"}
OBJS := ${patsubst ${SRCDIR}/%, ${BINDIR}/%, ${SRCS:.cpp=.o}}

CFLAGS := -std=c++17 -g -Wall -Wextra -Wpedantic -fsanitize=address
LDFLAGS := 

BINARY := ${BINDIR}/custom-cpu

.PHONY: test clean

run: ${BINARY}
	./$^

debug: ${BINARY}
	gdb $^

clean:
	rm -fr ${BINDIR}/*

${BINDIR}/%.o: ${SRCDIR}/%.cpp
	@mkdir -p $(@D)
	g++ ${CFLAGS} -c -o $@ $<

${BINARY}: ${OBJS}
	g++ ${CFLAGS} -o $@ $^ ${LDFLAGS}

compile_asmblr: assembly/main.cpp
	g++ -std=c++17 -g -Wall -Wextra -Wpedantic -fsanitize=address,undefined -o bin/assembler assembly/main.cpp

assemble: bin/assembler
	./$^ assembly.txt