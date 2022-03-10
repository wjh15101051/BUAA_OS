all: code/fibo.c code/main.c include/fibo.h
	gcc -I include -c code/fibo.c -o code/fibo.o
	gcc -I include -c code/main.c -o code/main.o
	gcc code/fibo.o code/main.o -o fibo
clean: code/fibo.o code/main.o
	rm code/fibo.o
	rm code/main.o
