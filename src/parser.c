
#include "utils.h"
#include "prog.h"
#include "prog.h"
#include <string.h> /* strlen */

static int c_is_digit(char c)
{
	return '0' <= c && c <= '9';
}

static int c_is_lowercase_letter(char c)
{
	return 'a' <= c && c <= 'z';
}

static int c_is_semicolon_instr(char c)
{
	return
		c_is_digit(c) || c_is_lowercase_letter(c) ||
		c == '+' || c == '-' || c == '*' || c == '/' || c == '%';
}

/* Returns the value of the pointed number literal.
 * The given index is updated. */
static unsigned int parse_number_literal(const char* src, unsigned int* index)
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
 * given word at the given index. The characters are parsed in short mode.
 * The given index is updated. */
static void parse_semicolon_word(const char* src, unsigned int* index,
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
		uint8_t* gsi_instr;
		#define GENERATE_SIMPLE_INSTR(instr_id_) \
			( \
				gsi_instr = prog_alloc(&PROG, 1), \
				gsi_instr[0] = instr_id_, \
				(void)0 \
			)
		char c = src[*index];
		int x;
		#define PCGSI(char_, instr_id_) \
			( \
				x = (c == (char_)), \
				(x ? \
					((*index)++, GENERATE_SIMPLE_INSTR(instr_id_)) \
				: (void)0), \
				x \
			)
		if (c_is_digit(c))
		{
			unsigned int value = parse_number_literal(src, index);
			ASSERT(value <= 255, "For now only bytes are supported\n");
			uint8_t* instr = prog_alloc(&PROG, 2);
			instr[0] = INSTR_ID_PUSH_IMM;
			instr[1] = value;
		}
		else if (PCGSI('n', INSTR_ID_NOP));
		else if (PCGSI('k', INSTR_ID_KILL));
		else if (PCGSI('d', INSTR_ID_DUPLICATE));
		else if (PCGSI('s', INSTR_ID_SWAP));
		else if (PCGSI('e', INSTR_ID_SET));
		else if (PCGSI('g', INSTR_ID_GET));
		else if (PCGSI('t', INSTR_ID_HEIGHT));
		else if (PCGSI('+', INSTR_ID_ADD));
		else if (PCGSI('-', INSTR_ID_SUBTRACT));
		else if (PCGSI('*', INSTR_ID_MULTIPLY));
		else if (PCGSI('/', INSTR_ID_DIVIDE));
		else if (PCGSI('%', INSTR_ID_MODULUS));
		else if (PCGSI('x', INSTR_ID_EXECUTE));
		else if (PCGSI('i', INSTR_ID_IFELSE));
		else if (PCGSI('w', INSTR_ID_DOWHILE));
		else if (PCGSI('r', INSTR_ID_REPEAT));
		else if (PCGSI('h', INSTR_ID_HALT));
		else if (PCGSI('p', INSTR_ID_PRINT_CHAR));
		else if (c_is_semicolon_instr(c))
		{
			ASSERT(0, "TODO: The semicolon instruction "
				"%c (%d) is not supported yet\n", c, (int)c);
		}
		else
		{
			break;
		}
		#undef PCGSI
		#undef GENERATE_SIMPLE_INSTR
		#undef PROG
	}
}

/* Compares the pointed word againts the given word in case of an exact match.
 * Returns non-zero when it is a match.
 * The given index is updated when it is a match. */
static int parse_word_match(const char* restrict src, unsigned int* index,
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

void parse_full_prog(const char* src, full_prog_t* full_prog)
{
	ASSERT(src != NULL, "The pointer is NULL\n");
	ASSERT_CHECK_FULL_PROG_PTR(full_prog);
	int short_mode_level = -1;
	unsigned int index = 0;
	unsigned int prog_index = full_prog_alloc_index(full_prog);
	char c;
	while ((c = src[index]) != '\0')
	{
		#define PROG full_prog->array[prog_index]
		uint8_t* gsi_instr;
		#define GENERATE_SIMPLE_INSTR(instr_id_) \
			( \
				gsi_instr = prog_alloc(&PROG, 1), \
				gsi_instr[0] = instr_id_, \
				(void)0 \
			)
		if (short_mode_level >= 0 && c_is_semicolon_instr(c))
		{
			parse_semicolon_word(src, &index, full_prog, prog_index);
		}
		else if (c == ';')
		{
			index++;
			if (src[index] == ';')
			{
				index++;
				if (short_mode_level >= 1)
				{
					ASSERT(0, "TODO: Error to say that "
						";; can't be used in a [ ] block already in ;;\n");
				}
				else if (short_mode_level == 0)
				{
					short_mode_level = -1;
				}
				else
				{
					short_mode_level = 0;
				}
			}
			else
			{
				parse_semicolon_word(src, &index, full_prog, prog_index);
			}
		}
		else if (c_is_digit(c))
		{
			unsigned int value = parse_number_literal(src, &index);
			ASSERT(value <= 255, "For now only bytes are supported\n");
			uint8_t* instr = prog_alloc(&PROG, 2);
			instr[0] = INSTR_ID_PUSH_IMM;
			instr[1] = value;
		}
		else if (c_is_lowercase_letter(c))
		{
			#define PWM1(word_) parse_word_match(src, &index, (word_))
			#define PWM2(word_1_, word_2_) (PWM1(word_1_) || PWM1(word_2_))
			int x;
			#define PWGSI(pw_code_, instr_id_) \
				( \
					x = (pw_code_), \
					(x ? GENERATE_SIMPLE_INSTR(instr_id_) : (void)0), \
					x \
				)
			if (0);
			else if (PWGSI(PWM1("nop"),              INSTR_ID_NOP));
			else if (PWGSI(PWM2("kil", "kill"),      INSTR_ID_KILL));
			else if (PWGSI(PWM2("dup", "duplicate"), INSTR_ID_DUPLICATE));
			else if (PWGSI(PWM2("swp", "swap"),      INSTR_ID_SWAP));
			else if (PWGSI(PWM1("set"),              INSTR_ID_SET));
			else if (PWGSI(PWM1("get"),              INSTR_ID_GET));
			else if (PWGSI(PWM2("hei", "height"),    INSTR_ID_HEIGHT));
			else if (PWGSI(PWM1("add"),              INSTR_ID_ADD));
			else if (PWGSI(PWM2("sub", "subtract"),  INSTR_ID_SUBTRACT));
			else if (PWGSI(PWM2("mul", "multiply"),  INSTR_ID_MULTIPLY));
			else if (PWGSI(PWM2("div", "divide"),    INSTR_ID_DIVIDE));
			else if (PWGSI(PWM2("mod", "modulus"),   INSTR_ID_MODULUS));
			else if (PWGSI(PWM2("exe", "execute"),   INSTR_ID_EXECUTE));
			else if (PWGSI(PWM2("ife", "ifelse"),    INSTR_ID_IFELSE));
			else if (PWGSI(PWM2("dwh", "dowhile"),   INSTR_ID_DOWHILE));
			else if (PWGSI(PWM2("rep", "repeat"),    INSTR_ID_REPEAT));
			else if (PWGSI(PWM2("hlt", "halt"),      INSTR_ID_HALT));
			else if (PWGSI(PWM2("pri", "print"),     INSTR_ID_PRINT_CHAR));
			else
			{
				ASSERT(0, "TODO: Error to say that a word "
					"starting by %c (%d) is unexpected\n", c, (int)c);
			}
			#undef PWGSI
			#undef PWM2
			#undef PWM1
		}
		else if (c == '\'')
		{
			index++;
			while (src[index] != '\'' && src[index] != '\0')
			{
				uint8_t* instr = prog_alloc(&PROG, 2);
				instr[0] = INSTR_ID_PUSH_IMM;
				instr[1] = src[index];
				index++;
			}
			if (src[index] == '\0')
			{
				fprintf(stderr, "Syntax warning: "
					"Non-closed single-quoted string\n");
				/* TODO: Setup a real logging system thing */
			}
			else
			{
				index++;
			}
		}
		else if (c == '[')
		{
			index++;
			unsigned int sub_prog_index = full_prog_alloc_index(full_prog);
			uint8_t* instr = prog_alloc(&PROG, 2);
			instr[0] = INSTR_ID_PUSH_IMM;
			instr[1] = sub_prog_index;
			prog_index = sub_prog_index;
			if (short_mode_level >= 0)
			{
				short_mode_level++;
			}
		}
		else if (c == ']')
		{
			PROG.is_finished = 1;
			while (PROG.is_finished)
			{
				prog_index--;
			}
			index++;
			if (short_mode_level >= 1)
			{
				short_mode_level--;
			}
			else if (short_mode_level == 0)
			{
				ASSERT(0, "TODO: Error to say that "
					";; must be closed before the [ ] block it is in\n");
			}
		}
		else if (c == '\t' || c == ' ' || c == '\n')
		{
			index++;
		}
		else if (c == '#')
		{
			index++;
			while (src[index] != '#' && src[index] != '\0')
			{
				index++;
			}
			if (src[index] == '\0')
			{
				fprintf(stderr, "Syntax warning: Non-closed comment\n");
				/* TODO: Setup a real logging system thing */
			}
			else
			{
				index++;
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
