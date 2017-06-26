#include <stdio.h>
#include "./hello/hello.h"

int main()
{
	printf("\n-->>>Test Kconfig\n\n");
#if CONFIG_HELLO_MAIN	
	printf("----->>> CONFIG_HELLO_MAIN\n");
#endif
	Say_Hello();
}