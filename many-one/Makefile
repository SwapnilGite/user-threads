CC = gcc

# The default target will be executed when you run "make" without specifying a target
default: compile_and_test

compile_and_test: clean compile run_tests

compile:
	$(CC) -c mythread.c test.c 

run_tests: test
	./test

test: test.o mythread.o
	$(CC) $^ -o test

clean:
	rm -f *.o test

