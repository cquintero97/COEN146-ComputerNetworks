// Christian Quintero
// Coen 146L
// Lab 1
// 4/5/18

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("Invalid number of arguements.");
		return 0;
	}
	
	FILE *sfp = fopen(argv[1], "rb");
	FILE *dfp = fopen(argv[2], "wb");
	
	if (sfp == NULL)
	{
		printf ("Unable to open file: file may be missing or invalid.");
		return 0;
	}
	
	char buffer[10];

	int temp = fread(buffer, sizeof(char), 10, sfp);
	while (temp > 0)
	{
		fwrite(buffer, sizeof(char),temp, dfp);
		temp = fread(buffer, sizeof(char), 10, sfp);
	}
	
	fclose(sfp);
	fclose(dfp);

	return 0;
		
} 
