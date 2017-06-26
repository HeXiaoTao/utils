#include <stdio.h>

// typedef struct __attribute__ ((packed))
// {
	// int  id;
	// char name[10];
	// char *p;
	// int  num;
// }	pBuf;

int Say_Hello()
{
#if CONFIG_HELLO_A	
	printf("----->>> CONFIG_HELLO_A\n");
#endif

#if CONFIG_HELLO_B	
	printf("----->>> CONFIG_HELLO_B\n");
#endif

	return 0;
}