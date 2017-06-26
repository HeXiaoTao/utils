ifdef DEBUG
hide :=
else
hide := @
endif

#all: run

inotify_test: inotify_test.c
	$(hide) gcc -g -Wall -Werror -O inotify_test.c -o inotify_test

run: inotify_test
	./inotify_test&
