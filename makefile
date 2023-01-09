all: column

column: column.c
	gcc -g -o column column.c -lm -Wall

run: all
	./column
