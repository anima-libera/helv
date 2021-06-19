
#include "utils.h"
#include "gs.h"
#include "prog.h"

/* Appends C code to the given growable string,
 * the generated C code corresponds to the given program. */
static void emit_c_prog(gs_t* gs, const prog_t* prog)
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
			case INSTR_ID_GET:
				EMIT("\tst[i-1] = st[st[i-1]];\n");
			break;
			case INSTR_ID_SET:
				EMIT("\tst[st[i-1]] = st[i-2]; i -= 2;\n");
			break;
			case INSTR_ID_HEIGHT:
				EMIT("\tst[i] = i; i++;\n");
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
