
#include "utils.h"
#include "gs.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h> /* strcmp */
#include <assert.h> /* static_assert */

/* Elementary macro instruction id, can and should fit in a byte. */
enum instr_id_t
{
	INSTR_ID_NOP = 0,
	INSTR_ID_PUSH_IMM, /* Immutable byte value follows. */
	INSTR_ID_KILL,
	INSTR_ID_DUPLICATE,
	INSTR_ID_SWAP,
	INSTR_ID_ADD,
	INSTR_ID_SUBTRACT,
	INSTR_ID_MULTIPLY,
	INSTR_ID_DIVIDE,
	INSTR_ID_MODULUS,
	INSTR_ID_EXECUTE,
	INSTR_ID_IFELSE,
	INSTR_ID_DOWHILE,
	INSTR_ID_REPEAT,
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

void execute_prog(const full_prog_t* full_prog, unsigned int prog_index,
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
			case INSTR_ID_DUPLICATE:
				EMIT("\tst[i] = st[i-1]; i++;\n");
			break;
			case INSTR_ID_SWAP:
				EMIT(
					"\t{"
						"uint8_t x = st[i-1]; "
						"st[i-1] = st[i-2]; "
						"st[i-2] = x;"
					"}\n");
			break;
			case INSTR_ID_ADD:
				EMIT("\tst[i-2] = st[i-1] + st[i-2]; i--;\n");
			break;
			case INSTR_ID_SUBTRACT:
				EMIT("\tst[i-2] = st[i-1] - st[i-2]; i--;\n");
			break;
			case INSTR_ID_MULTIPLY:
				EMIT("\tst[i-2] = st[i-1] * st[i-2]; i--;\n");
			break;
			case INSTR_ID_DIVIDE:
				EMIT("\tst[i-2] = st[i-1] / st[i-2]; i--;\n");
			break;
			case INSTR_ID_MODULUS:
				EMIT("\tst[i-2] = st[i-1] %% st[i-2]; i--;\n");
			break;
			case INSTR_ID_EXECUTE:
				EMIT("\tprog_table[st[--i]]();\n");
			break;
			case INSTR_ID_IFELSE:
				EMIT(
					"\tprog_table["
						"st[--i] ? "
						"(i--, st[i--]) : (i--, st[--i])" /* Oh my~ */
					"]();\n");
			break;
			case INSTR_ID_DOWHILE:
				EMIT(
					"\t{"
						"uint8_t f = st[--i]; "
						"do {"
							"prog_table[f]();"
						"} while (st[--i]);"
					"}\n");
			break;
			case INSTR_ID_REPEAT:
				EMIT(
					"\t{"
						"uint8_t n = st[--i]; "
						"uint8_t f = st[--i]; "
						"for (unsigned int j = 0; j < n; j++) {"
							"prog_table[f]();"
						"}"
					"}\n");
				/* TODO:
				 * Make it shorter so that it doesn't hit column 80. */
			break;
			case INSTR_ID_PRINT_CHAR:
				EMIT("\tputchar(st[--i]); fflush(stdout);\n");
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
	EMIT(
		"#include <stdlib.h>\n"
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
		EMIT("void prog_%u(void)\n", i);
		EMIT("{\n");
		emit_c_prog(gs, &full_prog->array[i]);
		EMIT("}\n");
	}
	EMIT(
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

int c_is_lowercase_letter(char c)
{
	return 'a' <= c && c <= 'z';
}

/* Returns the value of the pointed number literal.
 * The given index is updated. */
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
		#define PROG full_prog->array[prog_index]
		#define GENERATE_SIMPLE_INSTR(instr_id_) \
			do \
			{ \
				uint8_t* instr = prog_alloc(&PROG, 1); \
				instr[0] = instr_id_; \
			} while (0)
		char c = src[*index];
		if (c_is_digit(c))
		{
			unsigned int value = parse_number_literal(src, index);
			ASSERT(value <= 255, "For now only bytes are supported\n");
			uint8_t* instr = prog_alloc(&PROG, 2);
			instr[0] = INSTR_ID_PUSH_IMM;
			instr[1] = value;
		}
		else if (c == 'n')
		{
			GENERATE_SIMPLE_INSTR(INSTR_ID_NOP);
			(*index)++;
		}
		else if (c == 'k')
		{
			GENERATE_SIMPLE_INSTR(INSTR_ID_KILL);
			(*index)++;
		}
		else if (c == 'd')
		{
			GENERATE_SIMPLE_INSTR(INSTR_ID_DUPLICATE);
			(*index)++;
		}
		else if (c == 's')
		{
			GENERATE_SIMPLE_INSTR(INSTR_ID_SWAP);
			(*index)++;
		}
		else if (c == '+')
		{
			GENERATE_SIMPLE_INSTR(INSTR_ID_ADD);
			(*index)++;
		}
		else if (c == '-')
		{
			GENERATE_SIMPLE_INSTR(INSTR_ID_SUBTRACT);
			(*index)++;
		}
		else if (c == '*')
		{
			GENERATE_SIMPLE_INSTR(INSTR_ID_MULTIPLY);
			(*index)++;
		}
		else if (c == '/')
		{
			GENERATE_SIMPLE_INSTR(INSTR_ID_DIVIDE);
			(*index)++;
		}
		else if (c == '%')
		{
			GENERATE_SIMPLE_INSTR(INSTR_ID_MODULUS);
			(*index)++;
		}
		else if (c == 'x')
		{
			GENERATE_SIMPLE_INSTR(INSTR_ID_EXECUTE);
			(*index)++;
		}
		else if (c == 'i')
		{
			GENERATE_SIMPLE_INSTR(INSTR_ID_IFELSE);
			(*index)++;
		}
		else if (c == 'w')
		{
			GENERATE_SIMPLE_INSTR(INSTR_ID_DOWHILE);
			(*index)++;
		}
		else if (c == 'r')
		{
			GENERATE_SIMPLE_INSTR(INSTR_ID_REPEAT);
			(*index)++;
		}
		else if (c == 'h')
		{
			GENERATE_SIMPLE_INSTR(INSTR_ID_HALT);
			(*index)++;
		}
		else if (c == 'p')
		{
			GENERATE_SIMPLE_INSTR(INSTR_ID_PRINT_CHAR);
			(*index)++;
		}
		else
		{
			break;
		}
		#undef GENERATE_SIMPLE_INSTR
		#undef PROG
	}
}

/* Compares the pointed word againts the given word in case of an exact match.
 * Returns non-zero when it is a match.
 * The given index is updated when it is a match. */
int parse_word_match(const char* restrict src, unsigned int* index,
	const char* restrict word)
{
	ASSERT(src != NULL, "The pointer is NULL\n");
	ASSERT(index != NULL, "The pointer is NULL\n");
	ASSERT(*index <= strlen(src), "The source index is out of bounds\n");
	ASSERT(word != NULL, "The pointer is NULL\n");
	unsigned int i;
	for (i = 0; word[i] != '\0'; i++)
	{
		if (word[i] != src[(*index)+i])
		{
			return 0;
		}
	}
	if (!c_is_lowercase_letter(src[(*index)+i]))
	{
		*index += i;
		return 1;
	}
	else
	{
		return 0;
	}
}

/* Parse a program, being a full file or a [ ] block content. */
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
		#define PROG full_prog->array[prog_index]
		#define GENERATE_SIMPLE_INSTR(instr_id_) \
			do \
			{ \
				uint8_t* instr = prog_alloc(&PROG, 1); \
				instr[0] = instr_id_; \
			} while (0)
		if (c_is_digit(c))
		{
			unsigned int value = parse_number_literal(src, index);
			ASSERT(value <= 255, "For now only bytes are supported\n");
			uint8_t* instr = prog_alloc(&PROG, 2);
			instr[0] = INSTR_ID_PUSH_IMM;
			instr[1] = value;
		}
		else if (c_is_lowercase_letter(c))
		{
			#define PARSE_WORD(word_) parse_word_match(src, index, (word_))
			if (PARSE_WORD("nop"))
			{
				GENERATE_SIMPLE_INSTR(INSTR_ID_NOP);
			}
			else if (PARSE_WORD("kil") || PARSE_WORD("kill"))
			{
				GENERATE_SIMPLE_INSTR(INSTR_ID_KILL);
			}
			else if (PARSE_WORD("dup") || PARSE_WORD("duplicate"))
			{
				GENERATE_SIMPLE_INSTR(INSTR_ID_DUPLICATE);
			}
			else if (PARSE_WORD("swp") || PARSE_WORD("swap"))
			{
				GENERATE_SIMPLE_INSTR(INSTR_ID_SWAP);
			}
			else if (PARSE_WORD("add"))
			{
				GENERATE_SIMPLE_INSTR(INSTR_ID_ADD);
			}
			else if (PARSE_WORD("sub") || PARSE_WORD("subtract"))
			{
				GENERATE_SIMPLE_INSTR(INSTR_ID_SUBTRACT);
			}
			else if (PARSE_WORD("mul") || PARSE_WORD("multiply"))
			{
				GENERATE_SIMPLE_INSTR(INSTR_ID_MULTIPLY);
			}
			else if (PARSE_WORD("div") || PARSE_WORD("divide"))
			{
				GENERATE_SIMPLE_INSTR(INSTR_ID_DIVIDE);
			}
			else if (PARSE_WORD("mod") || PARSE_WORD("modulus"))
			{
				GENERATE_SIMPLE_INSTR(INSTR_ID_MODULUS);
			}
			else if (PARSE_WORD("exe") || PARSE_WORD("execute"))
			{
				GENERATE_SIMPLE_INSTR(INSTR_ID_EXECUTE);
			}
			else if (PARSE_WORD("ife") || PARSE_WORD("ifelse"))
			{
				GENERATE_SIMPLE_INSTR(INSTR_ID_IFELSE);
			}
			else if (PARSE_WORD("dwh") || PARSE_WORD("dowhile"))
			{
				GENERATE_SIMPLE_INSTR(INSTR_ID_DOWHILE);
			}
			else if (PARSE_WORD("rep") || PARSE_WORD("repeat"))
			{
				GENERATE_SIMPLE_INSTR(INSTR_ID_REPEAT);
			}
			else if (PARSE_WORD("hlt") || PARSE_WORD("halt"))
			{
				GENERATE_SIMPLE_INSTR(INSTR_ID_HALT);
			}
			else if (PARSE_WORD("pri") || PARSE_WORD("print"))
			{
				GENERATE_SIMPLE_INSTR(INSTR_ID_PRINT_CHAR);
			}
			else
			{
				ASSERT(0, "TODO: Error to say that a word "
					"starting by %c (%d) is unexpected\n", c, (int)c);
			}
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
			uint8_t* instr = prog_alloc(&PROG, 2);
			instr[0] = INSTR_ID_PUSH_IMM;
			instr[1] = sub_prog_index;
		}
		else if (c == '\t' || c == ' ' || c == '\n')
		{
			(*index)++;
		}
		else if (c == '#')
		{
			(*index)++;
			while (src[*index] != '#' && src[*index] != '\0')
			{
				(*index)++;
			}
			if (src[*index] == '\0')
			{
				fprintf(stderr, "Syntax warning: Non-closed comment\n");
				/* TODO: Setup a real logging system thing */
			}
			else
			{
				(*index)++;
			}
		}
		else
		{
			ASSERT(0, "TODO: Error to say %c (%d) is unexpected\n", c, (int)c);
		}
		#undef GENERATE_SIMPLE_INSTR
		#undef PROG
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

int main(int argc, const char** argv)
{
	const char* src = NULL;
	const char* dst = NULL;
	int src_is_allocated = 0;
	int help = 0;
	int version = 0;
	int execute = 0;

	for (unsigned int i = 1; i < (unsigned int)argc; i++)
	{
		if (argv[i][0] == '-')
		{
			#define IS(s1_, s2_) (strcmp((s1_), (s2_)) == 0)
			if (IS(argv[i], "-h") || IS(argv[i], "--help"))
			{
				help = 1;
			}
			else if (IS(argv[i], "-v") || IS(argv[i], "--version"))
			{
				version = 1;
			}
			else if (IS(argv[i], "-e") || IS(argv[i], "--execute"))
			{
				execute = 1;
			}
			else if (IS(argv[i], "-c") || IS(argv[i], "--code"))
			{
				if (i == (unsigned int)argc-1)
				{
					fprintf(stderr, "Command line argument error: "
						"The code option requiers a following argument\n");
				}
				else if (src != NULL)
				{
					fprintf(stderr, "Command line argument error: "
						"The code option cannot sets the source code "
						"as it is already given by previous arguments\n");
					i++;
				}
				else
				{
					src = argv[++i];
				}
			}
			else if (IS(argv[i], "-o") || IS(argv[i], "--out"))
			{
				if (i == (unsigned int)argc-1)
				{
					fprintf(stderr, "Command line argument error: "
						"The out option requiers a following argument\n");
				}
				else if (dst != NULL)
				{
					fprintf(stderr, "Command line argument error: "
						"The out option cannot sets the destination file "
						"as it is already given by previous arguments\n");
					i++;
				}
				else
				{
					dst = argv[++i];
				}
			}
			else
			{
				fprintf(stderr, "Command line argument error: "
					"Unknown option %s\n", argv[i]);
			}
			#undef IS
		}
		else /* Source code file name */
		{
			if (src != NULL)
			{
				fprintf(stderr, "Command line argument error: "
					"The file \"%s\" cannot be the source code "
					"as it is already given by previous arguments\n",
					argv[i]);
			}
			else
			{
				src = read_file(argv[i]);
				src_is_allocated = 1;
			}
		}
	}

	#ifdef DEBUG
		#define YN(condition_) ((condition_) ? "yes" : "no")
		printf("Debug build command line arguments:\n"
			"  Source code provided: %s\n"
			"  Compile or execute: %s\n"
			"  Destination file name: %s\n"
			"  Version wanted: %s\n"
			"  Help wanted: %s\n",
			YN(src != NULL),
			execute ? "execute" : "compile",
			dst != NULL ? dst : "*none*",
			YN(version),
			YN(help));
		#undef YN
		if (version || help || src != NULL)
		{
			printf("\n");
		}
	#endif

	#define VERSION_MAJOR 0
	#define VERSION_MINOR 0
	#define VERSION_PATCH 0
	#define VERSION_NAME "dev"

	if (version)
	{
		printf("Helv reference implementation, "
			"version %d.%d.%d %s, %s build\n",
			VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_NAME,
			#ifdef DEBUG
				"debug"
			#else
				"release"
			#endif
		);
		printf("See https://github.com/anima-libera/helv\n");
	}

	if (help)
	{
		if (version)
		{
			printf("\n");
		}
		printf(
			"Usage:\n"
			"  %s [options] file\n"
			"Options:\n"
			"  -c --code     Sets the program source to the next argument\n"
			"  -e --execute  Executes the program instead of compiling it\n"
			"  -h --help     Displays this help message\n"
			"  -o --out      Sets the output file name to the next argument\n"
			"  -v --version  Displays the implementation version\n",
			argc == 0 ? "helv" : argv[0]);
	}

	if (src == NULL)
	{
		return 0;
	}

	full_prog_t full_prog = {0};
	parse_full_prog(src, &full_prog);
	if (src_is_allocated)
	{
		free((char*)src);
	}

	if (execute)
	{
		st_t st = {0};
		execute_full_prog(&full_prog, &st);
		st_cleanup(&st);
	}
	else
	{
		gs_t gs;
		gs_init(&gs);
		emit_c_full_prog(&gs, &full_prog);
		if (dst != NULL)
		{
			FILE* dst_file = fopen(dst, "w");
			fputs(gs.str, dst_file);
			fclose(dst_file);
		}
		else
		{
			fputs(gs.str, stdout);
		}
		gs_cleanup(&gs);
	}

	full_prog_cleanup(&full_prog);

	return 0;
}
