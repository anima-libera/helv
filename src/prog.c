
#include "prog.h"
#include "utils.h"
#include <stdlib.h>
#include <stdint.h>

void prog_cleanup(prog_t* prog)
{
	ASSERT_CHECK_PROG_PTR(prog);
	free(prog->array);
}

uint8_t* prog_alloc(prog_t* prog, unsigned int len)
{
	ASSERT_CHECK_PROG_PTR(prog);
	prog->len += len;
	DARRAY_RESIZE_IF_NEEDED(prog->len, prog->cap, prog->array, uint8_t);
	return &prog->array[prog->len - len];
}

void full_prog_cleanup(full_prog_t* full_prog)
{
	ASSERT_CHECK_FULL_PROG_PTR(full_prog);
	for (unsigned int i = 0; i < full_prog->len; i++)
	{
		prog_cleanup(&full_prog->array[i]);
	}
	free(full_prog->array);
}

unsigned int full_prog_alloc_index(full_prog_t* full_prog)
{
	ASSERT_CHECK_FULL_PROG_PTR(full_prog);
	full_prog->len++;
	DARRAY_RESIZE_IF_NEEDED(full_prog->len, full_prog->cap,
		full_prog->array, prog_t);
	full_prog->array[full_prog->len-1] = (prog_t){0};
	return full_prog->len-1;
}
