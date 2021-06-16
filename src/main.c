
#include "utils.h"
#include "gs.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h> /* static_assert */

/* Elementary macro instruction id, can and should fit in a byte. */
enum instr_id_t
{
	INSTR_ID_NOP = 0,
	INSTR_ID_PUSH_IMM, /* Immutable byte value follows. */
	INSTR_ID_KILL,
	INSTR_ID_ADD,
	INSTR_ID_PRINT_CHAR,
	INSTR_ID_HALT,
	NUMBER_OF_INSTRUCTION_IDS
};
typedef enum instr_id_t instr_id_t;

static_assert(NUMBER_OF_INSTRUCTION_IDS <= 256,
	"There are too much instruction ids for them to fit in a byte");

/* A sequence of Helv instructions. */
struct prog_t
{
	unsigned int len;
	unsigned int cap;
	uint8_t* array; /* Bytecode. */
};
typedef struct prog_t prog_t;

#define ASSERT_CHECK_PROG_PTR(prog_ptr_) \
	do \
	{ \
		ASSERT(prog_ptr_ != NULL, "The pointer is NULL\n"); \
		ASSERT_CHECK_DARRAY(prog_ptr_->len, prog_ptr_->cap, \
			prog_ptr_->array); \
	} while (0)

void prog_cleanup(prog_t* prog)
{
	ASSERT_CHECK_PROG_PTR(prog);
	free(prog->array);
}

/* Extends the program by len uninitialized bytes,
 * and returns a pointer to the newly added bytes that must all be used. */
uint8_t* prog_alloc(prog_t* prog, unsigned int len)
{
	ASSERT_CHECK_PROG_PTR(prog);
	prog->len += len;
	DARRAY_RESIZE_IF_NEEDED(prog->len, prog->cap, prog->array, uint8_t);
	return &prog->array[prog->len - len];
}

/* Stack of unsigned bytes. */
struct st_t
{
	unsigned int len;
	unsigned int cap;
	uint8_t* array;
};
typedef struct st_t st_t;

#define ASSERT_CHECK_ST_PTR(st_ptr_) \
	do \
	{ \
		ASSERT(st_ptr_ != NULL, "The pointer is NULL\n"); \
		ASSERT_CHECK_DARRAY(st_ptr_->len, st_ptr_->cap, st_ptr_->array); \
	} while (0)

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

/* Full Helv program, as opposed to sub progras like those if [ ] blocks. */
struct full_prog_t
{
	unsigned int len;
	unsigned int cap;
	prog_t* array;
};
typedef struct full_prog_t full_prog_t;

#define ASSERT_CHECK_FULL_PROG_PTR(full_prog_ptr_) \
	do \
	{ \
		ASSERT(full_prog_ptr_ != NULL, "The pointer is NULL\n"); \
		ASSERT_CHECK_DARRAY(full_prog_ptr_->len, full_prog_ptr_->cap, \
			full_prog_ptr_->array); \
		CODE_FOR_ASSERT( \
			for (unsigned int i = 0; i < full_prog_ptr_->len; i++) \
			{ \
				prog_t* prog_ = &full_prog_ptr_->array[i]; \
				ASSERT_CHECK_PROG_PTR(prog_); \
			} \
		) \
	} while (0)

void full_prog_cleanup(full_prog_t* full_prog)
{
	ASSERT_CHECK_FULL_PROG_PTR(full_prog);
	for (unsigned int i = 0; i < full_prog->len; i++)
	{
		prog_cleanup(&full_prog->array[i]);
	}
	free(full_prog->array);
}

/* Adds an empty program to the given full program,
 * and returns the new program's index. */
unsigned int full_prog_alloc_index(full_prog_t* full_prog)
{
	ASSERT_CHECK_FULL_PROG_PTR(full_prog);
	full_prog->len++;
	DARRAY_RESIZE_IF_NEEDED(full_prog->len, full_prog->cap,
		full_prog->array, prog_t);
	full_prog->array[full_prog->len-1] = (prog_t){0};
	return full_prog->len-1;
}

void execute_prog(const prog_t* prog, st_t* st)
{
	ASSERT_CHECK_PROG_PTR(prog);
	ASSERT_CHECK_ST_PTR(st);
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
			case INSTR_ID_ADD:
				st_push(st, st_pop(st) + st_pop(st));
			break;
			case INSTR_ID_PRINT_CHAR:
				putchar(st_pop(st));
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
	execute_prog(&full_prog->array[0], st);
}

/* Appends C code to the given growable string,
 * the generated C code corresponds to the given program. */
void emit_c_prog(gs_t* gs, const prog_t* prog)
{
	ASSERT_CHECK_GS_PTR(gs);
	ASSERT_CHECK_PROG_PTR(prog);
	#define EMIT(...) gs_append_f(gs, __VA_ARGS__)
	unsigned int i = 0;
	while (i < prog->len)
	{
		switch (prog->array[i++])
		{
			case INSTR_ID_NOP:
				EMIT("\t;\n");
			break;
			case INSTR_ID_PUSH_IMM:
				ASSERT(i < prog->len,
					"A \"push immediate\" instruction cannot start "
					"at the last byte\n");
				EMIT("\tst[i++] = %u;\n", (unsigned int)prog->array[i++]);
			break;
			case INSTR_ID_KILL:
				EMIT("\ti--;\n");
			break;
			case INSTR_ID_ADD:
				EMIT("\tst[i-2] += st[i-1]; i--;\n");
			break;
			case INSTR_ID_PRINT_CHAR:
				EMIT("\tputchar(st[--i]);\n");
			break;
			case INSTR_ID_HALT:
				EMIT("\texit(0);\n");
			break;
		}
	}
	#undef EMIT
}

void emit_c_full_prog(gs_t* gs, const full_prog_t* full_prog)
{
	ASSERT_CHECK_GS_PTR(gs);
	ASSERT_CHECK_FULL_PROG_PTR(full_prog);
	ASSERT(full_prog->len >= 1,
		"The full program does not contain even one program\n");
	#define EMIT(...) gs_append_f(gs, __VA_ARGS__)
	EMIT(""
		"#include <stdio.h>\n"
		"#include <stdint.h>\n"
		"uint8_t st[99999];\n"
		"unsigned int i = 0;\n");
	for (unsigned int i = 0; i < full_prog->len; i++)
	{
		EMIT("void prog_%u(void);\n", i);
	}
	EMIT("void (*prog_table[])(void) = {\n");
	for (unsigned int i = 0; i < full_prog->len; i++)
	{
		EMIT("\tprog_%u%s\n", i, i < full_prog->len-1 ? "," : "");
	}
	EMIT("};\n");
	for (unsigned int i = 0; i < full_prog->len; i++)
	{
		EMIT("int prog_%u(void)\n", i);
		EMIT("{\n");
		emit_c_prog(gs, &full_prog->array[i]);
		EMIT("}\n");
	}
	EMIT(""
		"int main(void)\n"
		"{\n"
		"\tprog_table[0]();\n"
		"}\n");
	#undef EMIT
}

int c_is_digit(char c)
{
	return '0' <= c && c <= '9';
}

unsigned int parse_number_literal(const char* src, unsigned int* index)
{
	ASSERT(src != NULL, "The pointer is NULL\n");
	ASSERT(index != NULL, "The pointer is NULL\n");
	ASSERT(*index <= strlen(src), "The source index is out of bounds\n");
	char c;
	unsigned int value = 0;
	while (c_is_digit(c = src[*index]))
	{
		value *= 10;
		value += c - '0';
		(*index)++;
	}
	return value;
}

/* Adds to the end of the given program the instructions represented by the
 * given word at the given index. The character just before the pointed word
 * should be a semicolon.
 * The given index is updated. */
void parse_semicolon_word(const char* src, unsigned int* index,
	full_prog_t* full_prog, unsigned int prog_index)
{
	ASSERT(src != NULL, "The pointer is NULL\n");
	ASSERT(index != NULL, "The pointer is NULL\n");
	ASSERT(*index <= strlen(src), "The source index is out of bounds\n");
	ASSERT_CHECK_FULL_PROG_PTR(full_prog);
	ASSERT(prog_index < full_prog->len,
		"The program index is out of bounds\n");
	while (1)
	{
		char c = src[*index];
		if (c_is_digit(c))
		{
			unsigned int value = parse_number_literal(src, index);
			ASSERT(value <= 255, "For now only bytes are supported\n");
			uint8_t* instr = prog_alloc(&full_prog->array[prog_index], 2);
			instr[0] = INSTR_ID_PUSH_IMM;
			instr[1] = value;
		}
		else if (c == 'n')
		{
			uint8_t* instr = prog_alloc(&full_prog->array[prog_index], 1);
			instr[0] = INSTR_ID_NOP;
			(*index)++;
		}
		else if (c == 'k')
		{
			uint8_t* instr = prog_alloc(&full_prog->array[prog_index], 1);
			instr[0] = INSTR_ID_KILL;
			(*index)++;
		}
		else if (c == '+')
		{
			uint8_t* instr = prog_alloc(&full_prog->array[prog_index], 1);
			instr[0] = INSTR_ID_ADD;
			(*index)++;
		}
		else if (c == 'h')
		{
			uint8_t* instr = prog_alloc(&full_prog->array[prog_index], 1);
			instr[0] = INSTR_ID_HALT;
			(*index)++;
		}
		else if (c == 'p')
		{
			uint8_t* instr = prog_alloc(&full_prog->array[prog_index], 1);
			instr[0] = INSTR_ID_PRINT_CHAR;
			(*index)++;
		}
		else
		{
			break;
		}
	}
}

void parse_prog(const char* src, unsigned int* index,
	full_prog_t* full_prog, unsigned int prog_index,
	int end_is_eof)
{
	ASSERT(src != NULL, "The pointer is NULL\n");
	ASSERT(index != NULL, "The pointer is NULL\n");
	ASSERT(*index <= strlen(src), "The source index is out of bounds\n");
	ASSERT_CHECK_FULL_PROG_PTR(full_prog);
	ASSERT(prog_index < full_prog->len,
		"The program index is out of bounds\n");
	char end_char = end_is_eof ? '\0' : ']';
	char c;
	while ((c = src[*index]) != end_char)
	{
		if (c_is_digit(c))
		{
			unsigned int value = parse_number_literal(src, index);
			ASSERT(value <= 255, "For now only bytes are supported\n");
			uint8_t* instr = prog_alloc(&full_prog->array[prog_index], 2);
			instr[0] = INSTR_ID_PUSH_IMM;
			instr[1] = value;
		}
		else if (c == ';')
		{
			(*index)++;
			parse_semicolon_word(src, index, full_prog, prog_index);
		}
		else if (c == '[')
		{
			(*index)++;
			unsigned int sub_prog_index = full_prog_alloc_index(full_prog);
			parse_prog(src, index, full_prog, sub_prog_index, 0);
			(*index)++; /* Skip the ']'. */
			uint8_t* instr = prog_alloc(&full_prog->array[prog_index], 2);
			instr[0] = INSTR_ID_PUSH_IMM;
			instr[1] = sub_prog_index;
		}
		else if (c == '\t' || c == ' ' || c == '\n')
		{
			(*index)++;
		}
		else
		{
			ASSERT(0, "TODO: Error to say %c (%d) is unexpected\n", c, (int)c);
		}
	}
}

void parse_full_prog(const char* src, full_prog_t* full_prog)
{
	ASSERT(src != NULL, "The pointer is NULL\n");
	ASSERT_CHECK_FULL_PROG_PTR(full_prog);
	unsigned int index = 0;
	unsigned int prog_index = full_prog_alloc_index(full_prog);
	parse_prog(src, &index, full_prog, prog_index, 1);
}

int main(void)
{
	full_prog_t full_prog = {0};

	parse_full_prog("  10 96[;h];+p \t\n  ;ph", &full_prog);

	gs_t gs;
	gs_init(&gs);

	emit_c_full_prog(&gs, &full_prog);

	st_t st = {0};
	execute_full_prog(&full_prog, &st);
	st_cleanup(&st);

	full_prog_cleanup(&full_prog);

	fputs(gs.str, stdout);
	gs_cleanup(&gs);

	return 0;
}
