all: code/fibo.c code/main.c include/fibo.h
	$(MAKE) --directory="code"
	gcc code/fibo.o code/main.o -o fibo
clean: code/fibo.o code/main.o
	$(MAKE) clean --directory="code"
