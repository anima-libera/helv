
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

unsigned int umax(unsigned int a, unsigned int b)
{
	return a < b ? b : a;
}

char* read_file(const char* file_path)
{
	FILE* file = fopen(file_path, "r");
	if (file == NULL)
	{
		fprintf(stderr, "File error: failed to open \"%s\"\n", file_path);
		/* TODO: read errno, etc. */
		return NULL;
	}
	fseek(file, 0, SEEK_END);
	unsigned int len = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* buffer = xmalloc(len+1);
	if (fread(buffer, 1, len, file) < len)
	{
		fprintf(stderr, "File error: TODO: Hanle fread error\n");
		/* TODO: check with feof, read errno, etc. */
		return NULL;
	}
	buffer[len] = '\0';
	fclose(file);
	return buffer;
}
