#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(int argc, const char* argv[])
{
	if(argc < 2) return 0;

	while(1)
	{
		printf("%s\n", argv[1]);
		sleep(1);
	}
}
