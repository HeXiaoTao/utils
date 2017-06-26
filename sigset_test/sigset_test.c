/*
 * sigset test.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>

static void die(const char *fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
	fputc('\n', stderr);
	exit(1);
}

int main()
{
	int s, sig = 0;
	sigset_t set;
	sigemptyset(&set);

	sigaddset(&set, SIGALRM);
	pthread_sigmask(SIG_SETMASK, &set, NULL);

	for(;;) {
		s = sigwait(&set, &sig);
		if (s != 0)
			die("sigwait: %s", strerror(s));
		switch(sig) {
		case SIGALRM:
			printf("sigwait, receive signal SIGALRM\n");
			/*do the job when catch the sigwait*/
		break;
		default:
			break;
		}
	}

	/*
	sigdelset(&set, SIGALRM);
	pthread_sigmask(SIG_SETMASK, &set, NULL);
	*/

	return 0;
}
