main: main.c
	clang --std=c23 -Wall -Werror -o main main.c

.PHONY: run

run: main
	./main
