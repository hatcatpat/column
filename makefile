all: column

column: column.c
	gcc -o column column.c -lm -Wall

run: all
	./column

clean:
	rm column
