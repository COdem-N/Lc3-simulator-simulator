#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    FILE *file;
	unsigned int mem;
    char str[80];
    int i = 0;
	char* temp;
	char str3[] = "test.hex";


    file = fopen(str3, "r");

    if (file)
    {
		  printf("file opened %s\n",str3);

	while (fread(str, 1, 6, file) == 6)
	{
			
	    mem = strtol(str, &temp, 16);
	    printf("%s\n", str);

		printf("mem = x%X\n",mem);
	    i++;

	}
    }
    else
    {
	printf("erreor file not found");
    }
}
