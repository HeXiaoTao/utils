hide := @

#all: run

sigset_test: sigset_test.c
	$(hide) gcc -g -Wall -O sigset_test.c -o sigset_test -lpthread

run: sigset_test
	./sigset_test&
