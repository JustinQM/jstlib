main: main.c test.c
	clang --std=c23 -Wall -Werror -o main main.c test.c

.PHONY: run

run: main
	./main
