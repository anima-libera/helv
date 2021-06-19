
#include "utils.h"
#include "interpreter.h"
#include <stdint.h>

void st_cleanup(st_t* st)
{
	ASSERT_CHECK_ST_PTR(st);
	free(st->array);
}

void st_push(st_t* st, uint8_t byte)
{
	ASSERT_CHECK_ST_PTR(st);
	st->len++;
	DARRAY_RESIZE_IF_NEEDED(st->len, st->cap, st->array, uint8_t);
	st->array[st->len-1] = byte;
}

uint8_t st_pop(st_t* st)
{
	ASSERT_CHECK_ST_PTR(st);
	ASSERT(st->len >= 1, "The stack is empty, there is nothing to pop\n");
	return st->array[--st->len];
}

static void execute_prog(const full_prog_t* full_prog, unsigned int prog_index,
	st_t* st)
{
	ASSERT_CHECK_FULL_PROG_PTR(full_prog);
	ASSERT_CHECK_ST_PTR(st);
	ASSERT(prog_index < full_prog->len,
		"Attempting to execute out of the program table bounds\n");
	const prog_t* prog = &full_prog->array[prog_index];
	unsigned int i = 0;
	while (i < prog->len)
	{
		switch (prog->array[i++])
		{
			case INSTR_ID_NOP:
				;
			break;
			case INSTR_ID_PUSH_IMM:
				ASSERT(i < prog->len,
					"A \"push immediate\" instruction cannot start "
					"at the last byte\n");
				st_push(st, prog->array[i++]);
			break;
			case INSTR_ID_KILL:
				st_pop(st);
			break;
			case INSTR_ID_DUPLICATE:
				{
					uint8_t a = st_pop(st);
					st_push(st, a);
					st_push(st, a);
				}
			break;
			case INSTR_ID_SWAP:
				{
					uint8_t a = st_pop(st);
					uint8_t b = st_pop(st);
					st_push(st, a);
					st_push(st, b);
				}
			break;
			case INSTR_ID_GET:
				{
					uint8_t index = st_pop(st);
					ASSERT(index < st->len,
						"Attempting to get out of bounds\n");
					st_push(st, st->array[index]);
				}
			break;
			case INSTR_ID_SET:
				{
					uint8_t index = st_pop(st);
					uint8_t value = st_pop(st);
					ASSERT(index < st->len,
						"Attempting to set out of bounds\n");
					st->array[index] = value;
				}
			break;
			case INSTR_ID_HEIGHT:
				st_push(st, st->len);
			break;
			case INSTR_ID_ADD:
				st_push(st, st_pop(st) + st_pop(st));
			break;
			case INSTR_ID_SUBTRACT:
				{
					uint8_t a = st_pop(st);
					uint8_t b = st_pop(st);
					st_push(st, a - b);
				}
			break;
			case INSTR_ID_MULTIPLY:
				st_push(st, st_pop(st) * st_pop(st));
			break;
			case INSTR_ID_DIVIDE:
				{
					uint8_t a = st_pop(st);
					uint8_t b = st_pop(st);
					ASSERT(b != 0, "Attempting to devide by zero\n");
					st_push(st, a / b);
				}
			break;
			case INSTR_ID_MODULUS:
				{
					uint8_t a = st_pop(st);
					uint8_t b = st_pop(st);
					ASSERT(b != 0,
						"Attempting to get the reminder "
						"of a division by zero\n");
					st_push(st, a % b);
				}
			break;
			case INSTR_ID_EXECUTE:
				{
					uint8_t sub_prog_index = st_pop(st);
					ASSERT(sub_prog_index < full_prog->len,
						"Attempting to execute "
						"out of the program table bounds\n");
					execute_prog(full_prog, sub_prog_index, st);
				}
			break;
			case INSTR_ID_IFELSE:
				{
					uint8_t condition = st_pop(st);
					uint8_t if_prog_index = st_pop(st);
					uint8_t else_prog_index = st_pop(st);
					uint8_t chosen_prog_index =
						condition ? if_prog_index : else_prog_index;
					ASSERT(chosen_prog_index < full_prog->len,
						"Attempting to ifelse-execute "
						"out of the program table bounds\n");
					execute_prog(full_prog, chosen_prog_index, st);
				}
			break;
			case INSTR_ID_DOWHILE:
				{
					uint8_t dowhile_prog_index = st_pop(st);
					ASSERT(dowhile_prog_index < full_prog->len,
						"Attempting to dowhile-execute "
						"out of the program table bounds\n");
					/* Hope that if one day the program table can be shrinked
					 * dynamically, then this ASSERT gets moved in the do while
					 * loop. */
					do
					{
						execute_prog(full_prog, dowhile_prog_index, st);
					} while (st_pop(st) != 0);
				}
			break;
			case INSTR_ID_REPEAT:
				{
					uint8_t how_may_times = st_pop(st);
					uint8_t repeat_prog_index = st_pop(st);
					ASSERT(how_may_times > 0 &&
						repeat_prog_index < full_prog->len,
						"Attempting to repeat-execute "
						"out of the program table bounds\n");
					/* Hope that if one day the program table can be shrinked
					 * dynamically, then this ASSERT gets moved in the for
					 * loop. */
					for (unsigned int j = 0; j < how_may_times; j++)
					{
						execute_prog(full_prog, repeat_prog_index, st);
					}
				}
			break;
			case INSTR_ID_PRINT_CHAR:
				putchar(st_pop(st));
				fflush(stdout);
			break;
			case INSTR_ID_HALT:
				return;
			break;
		}
	}
}

void execute_full_prog(const full_prog_t* full_prog, st_t* st)
{
	ASSERT_CHECK_FULL_PROG_PTR(full_prog);
	ASSERT_CHECK_ST_PTR(st);
	ASSERT(full_prog->len >= 1,
		"The full program does not contain even one program\n");
	execute_prog(full_prog, 0, st);
}
