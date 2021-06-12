
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

int g_debug = 0;

#define xmalloc malloc
#define xcalloc calloc
#define xrealloc realloc

#ifdef DEBUG
#define ENABLE_ASSERT
#endif

#ifdef ENABLE_ASSERT
#define ASSERT(condition_) ((condition_) ? ((void)0) : (void)fprintf(stderr, \
	"Assertion failed: At line %d in function %s in file " __FILE__ "\n", \
	__LINE__, __func__))
#else
#define ASSERT(condition_) ((void)0)
#endif

static unsigned int umax(unsigned int a, unsigned int b)
{
	return a < b ? b : a;
}

#define ARRAY_RESIZE_IF_NEEDED(len_, cap_, ptr_, elem_type_) \
	do { \
		if (len_ > cap_) \
		{ \
			unsigned int new_cap_wanted_ = umax(len_, \
				((float)cap_ + 2.3f) * 1.6f); \
			elem_type_* new_array_ = xrealloc(ptr_, \
				new_cap_wanted_ * sizeof(elem_type_)); \
			ASSERT(new_array_ != NULL); \
			ptr_ = new_array_; \
			cap_ = new_cap_wanted_; \
		} \
	} while (0)

enum instr_name_t
{
	INSTR_NOP = 0,
	INSTR_GLOBAL_HALT,
	INSTR_LOCAL_HALT,
	INSTR_PUSH_IMMUINT,
	INSTR_SWAP_ANY,
	INSTR_DUPLICATE_ANY,
	INSTR_KILL_ANY,
	INSTR_ADD_UINT,
	INSTR_SUBTRACT_UINT,
	INSTR_MULTIPLY_UINT,
	INSTR_DIVIDE_UINT,
	INSTR_MODULUS_UINT,
	INSTR_PRINT_UINT_AS_CHAR,
	INSTR_PRINT_UINT, /* TODO: remove and put in the stdlib */
	INSTR_PRINT_STACK, /* TODO: remove and put in the stdlib */
	INSTR_EXECUTE_CODEINDEX,
	INSTR_IFELSE_CODEINDEX,
	INSTR_DOWHILE_CODEINDEX,
};
typedef enum instr_name_t instr_name_t;

/* Strings instruction name */
struct sin_t
{
	char* str;
	char* str3; /* Three-chars abreviation of the classic name, can be NULL */
	char one_char; /* For the ";"/";;" mode, none is '\0' */
	instr_name_t name;
};
typedef struct sin_t sin_t;

struct sin_map_t
{
	unsigned int len;
	unsigned int cap;
	sin_t* array;
};
typedef struct sin_map_t sin_map_t;

sin_t* sin_map_alloc_sin(sin_map_t* map)
{
	map->len++;
	ARRAY_RESIZE_IF_NEEDED(map->len, map->cap, map->array, sin_t);
	return &map->array[map->len-1];
}

struct instr_t
{
	instr_name_t name;
	union
	{
		uint64_t uint64;
		char* string;
	};
};
typedef struct instr_t instr_t;

struct prog_t
{
	unsigned int len;
	unsigned int cap;
	instr_t* array;
};
typedef struct prog_t prog_t;

instr_t* prog_alloc_instr(prog_t* prog)
{
	prog->len++;
	ARRAY_RESIZE_IF_NEEDED(prog->len, prog->cap, prog->array, instr_t);
	return &prog->array[prog->len-1];
}

/* Growable string. */
struct gs_t
{
	unsigned int len; /* Including the null terminator. */
	unsigned int cap;
	char* str;
};
typedef struct gs_t gs_t;

void gs_init(gs_t* gs)
{
	gs->str = malloc(1);
	gs->str[0] = '\0';
	gs->len = 1;
	gs->cap = 1;
}

void gs_cleanup(gs_t* gs)
{
	free(gs->str);
}

/* Appends the printf-formatted arguments to the given growable string. */
void gs_append_f(gs_t* gs, const char* format, ...)
{
	va_list ap_1, ap_2;
	va_start(ap_1, format);
	va_copy(ap_2, ap_1);
	unsigned int available_len = gs->cap - gs->len;
	unsigned int requested_len = vsnprintf(&gs->str[gs->len-1],
		available_len, format, ap_1);
	va_end(ap_1);
	if (requested_len >= available_len)
	{
		gs->len += requested_len;
		ARRAY_RESIZE_IF_NEEDED(gs->len, gs->cap, gs->str, char);
		gs->len -= requested_len;
		vsprintf(&gs->str[gs->len-1], format, ap_2);
	}
	va_end(ap_2);
	gs->len += requested_len;
}

char* copy_word(const char* src, unsigned int* len)
{
	unsigned int i = 0;
	while ('a' <= src[i] && src[i] <= 'z')
	{
		i++;
	}
	char* word = malloc(i+1);
	for (unsigned int j = 0; j < i; j++)
	{
		word[j] = src[j];
	}
	word[i] = '\0';
	if (len != NULL)
	{
		*len = i;
	}
	return word;
}

int str_startswith(const char* str, const char* pattern)
{
	for (unsigned int i = 0; pattern[i] != '\0'; i++)
	{
		if (str[i] != pattern[i])
		{
			return 0;
		}
	}
	return 1;
}

struct prog_table_t
{
	unsigned int len;
	unsigned int cap;
	prog_t* array;
};
typedef struct prog_table_t prog_table_t;

unsigned int prog_table_alloc_prog_index(prog_table_t* table)
{
	table->len++;
	ARRAY_RESIZE_IF_NEEDED(table->len, table->cap, table->array, prog_t);
	return table->len-1;
}

/* Parsing mode set. */
struct pms_t
{
	unsigned int is_pop_after_use: 1; /* The `;` syntax instead of `;;`. */
	unsigned int has_basic_short: 1;
	unsigned int has_basic_short_math: 1;
};
typedef struct pms_t pms_t;

void pms_init(pms_t* pms)
{
	*pms = (pms_t){0};
}

void pms_copy(const pms_t* pms_src, pms_t* pms_dts)
{
	*pms_dts = *pms_src;
}

void pms_cleanup(pms_t* pms)
{
	(void)pms;
}

struct pms_st_t
{
	unsigned int len;
	unsigned int cap;
	pms_t* array;
};
typedef struct pms_st_t pms_st_t;

pms_t* pms_st_push_init(pms_st_t* pms_st)
{
	pms_st->len++;
	ARRAY_RESIZE_IF_NEEDED(pms_st->len, pms_st->cap, pms_st->array, pms_t);
	pms_init(&pms_st->array[pms_st->len-1]);
	return &pms_st->array[pms_st->len-1];
}

pms_t* pms_st_duplicate(pms_st_t* pms_st)
{
	pms_st->len++;
	ARRAY_RESIZE_IF_NEEDED(pms_st->len, pms_st->cap, pms_st->array, pms_t);
	pms_copy(&pms_st->array[pms_st->len-2], &pms_st->array[pms_st->len-1]);
	return &pms_st->array[pms_st->len-1];
}

void pms_st_pop(pms_st_t* pms_st)
{
	pms_st->len--;
	pms_cleanup(&pms_st->array[pms_st->len]);
}

void pms_st_cleanup(pms_st_t* pms_st)
{
	while (pms_st->len > 0)
	{
		pms_st_pop(pms_st);
	}
}

int c_is_math_operator(char c)
{
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '%';
}

int c_is_decimal_digit(char c)
{
	return '0' <= c && c <= '9';
}

int c_is_lowercase(char c)
{
	return 'a' <= c && c <= 'z';
}

int c_is_name_char(char c)
{
	return c_is_lowercase(c) || c == '-';
}

/* Read-only stream that seems to have infinitly many '\0' on the right. */
struct rs_t
{
	const char* str;
	unsigned int len; /* Excluding the null terminator. */
	union {
		unsigned int head_index;
		unsigned int i;
	};
};
typedef struct rs_t rs_t;

void rs_init(rs_t* rs, const char* str)
{
	rs->str = str;
	rs->len = strlen(str);
	rs->head_index = 0;
}

void rs_cleanup(rs_t* rs)
{
	(void)rs;
}

/* Gets the character at index head_index + head_offset. */
char rs_peek(const rs_t* rs, int head_offset)
{
	int index = (int)rs->head_index + head_offset;
	ASSERT(index >= 0);
	if ((unsigned int)index >= rs->len)
	{
		return '\0';
	}
	else
	{
		return rs->str[index];
	}
}

int rs_startswith(const rs_t* rs, const char* pattern)
{
	return str_startswith(&rs->str[rs->head_index], pattern);
}

unsigned int rs_word_len(const rs_t* rs, int (*c_is_word_char)(char c))
{
	unsigned int len = 0;
	while (c_is_word_char(rs->str[rs->head_index + len]))
	{
		len++;
	}
	return len;
}

int rs_startswith_parsing_mode(const rs_t* rs, unsigned int* name_len)
{
	return rs_peek(rs, (*name_len = rs_word_len(rs, c_is_lowercase))) == ';';
}

enum prog_type_t
{
	PROG_FILE,
	PROG_BRACKETS,
};
typedef enum prog_type_t prog_type_t;

/* Parse the given string as a program, such as the content of a [ ] block.
 * Returns non-zero on error. */
int parse_prog(rs_t* src_rs, unsigned int prog_index, prog_table_t* table,
	prog_type_t prog_type, pms_st_t* pms_st, sin_map_t* sinm)
{
	#define TOP_PMS pms_st->array[pms_st->len-1]
	while (1)
	{
		unsigned int a;
		char c = rs_peek(src_rs, 0);
		if (rs_startswith_parsing_mode(src_rs, &a))
		{
			if (rs_peek(src_rs, a + 1) != ';') /* Ends with only one ";" */
			{
				if (!TOP_PMS.is_pop_after_use)
				{
					pms_st_duplicate(pms_st);
					TOP_PMS.is_pop_after_use = 1;
				}
				src_rs->i += a + 1;
			}
			else
			{
				src_rs->i++;
			}
			if (a == 0)
			{
				TOP_PMS.has_basic_short = 1;
			}
			else if (a == 1 && c == 'm')
			{
				TOP_PMS.has_basic_short = 1;
				TOP_PMS.has_basic_short_math = 1;
			}
		}
		else if (c_is_lowercase(c)
			|| (TOP_PMS.has_basic_short_math && c_is_math_operator(c)))
		{
			if (TOP_PMS.has_basic_short) /* Implied by short math */
			{
				for (unsigned int i = 0; i < sinm->len; i++)
				{
					if (c == sinm->array[i].one_char)
					{
						*prog_alloc_instr(&table->array[prog_index]) = 
							(instr_t){.name = sinm->array[i].name};
						src_rs->i++;
						break;
					}
				}
			}
			else
			{
				for (unsigned int i = 0; i < sinm->len; i++)
				{
					if (rs_startswith(src_rs, sinm->array[i].str))
					{
						*prog_alloc_instr(&table->array[prog_index]) = 
							(instr_t){.name = sinm->array[i].name};
						src_rs->i += strlen(sinm->array[i].str);
						/* TODO: Optimize away this stupid strlen call! */
						break;
					}
					else if (sinm->array[i].str3 != NULL
						&& rs_startswith(src_rs, sinm->array[i].str3))
					{
						*prog_alloc_instr(&table->array[prog_index]) = 
							(instr_t){.name = sinm->array[i].name};
						src_rs->i += 3;
						break;
					}
				}
			}
		}
		else if (c == '[')
		{
			int pop_pms_after = 0;
			if (TOP_PMS.is_pop_after_use)
			{
				pop_pms_after = 1;
				TOP_PMS.is_pop_after_use = 0;
			}
			unsigned int sub_prog_index = prog_table_alloc_prog_index(table);
			table->array[sub_prog_index] = (prog_t){0};
			src_rs->i++;
			if (parse_prog(src_rs, sub_prog_index, table,
				PROG_BRACKETS, pms_st, sinm) != 0)
			{
				return -1;
			}
			if (rs_peek(src_rs, 0) == ']')
			{
				/* TODO: Change this ? Maybe ? */
				src_rs->i++;
			}
			if (pop_pms_after)
			{
				pms_st_pop(pms_st);
			}
			*prog_alloc_instr(&table->array[prog_index]) = (instr_t){
				.name = INSTR_PUSH_IMMUINT, .uint64 = sub_prog_index
			};
		}
		else if (c == ']')
		{
			if (TOP_PMS.is_pop_after_use)
			{
				pms_st_pop(pms_st);
			}
			if (prog_type == PROG_FILE)
			{
				fprintf(stderr, "Syntax warning: Too much closed brackets\n");
			}
			break;
		}
		else if (c_is_decimal_digit(c))
		{
			unsigned int imm_value = 0;
			char d;
			while (c_is_decimal_digit(d = rs_peek(src_rs, 0)))
			{
				imm_value = imm_value * 10 + (d - '0');
				src_rs->i++;
			}
			*prog_alloc_instr(&table->array[prog_index]) = (instr_t){
				.name = INSTR_PUSH_IMMUINT, .uint64 = imm_value
			};
		}
		else if (c == ' ' || c == '\n' || c == '\t')
		{
			if (TOP_PMS.is_pop_after_use)
			{
				pms_st_pop(pms_st);
			}
			src_rs->i++;
		}
		else if (c == '#')
		{
			src_rs->i++;
			while (rs_peek(src_rs, 0) != '#' && rs_peek(src_rs, 0) != '\0')
			{
				src_rs->i++;
			}
			if (rs_peek(src_rs, 0) == '\0')
			{
				fprintf(stderr, "Syntax warning: Non terminated comment\n");
			}
			else
			{
				src_rs->i++;
			}
		}
		else if (c == '\0')
		{
			if (prog_type == PROG_BRACKETS)
			{
				fprintf(stderr, "Syntax warning: Non closed brackets\n");
			}
			break;
		}
		else if (c_is_math_operator(c))
		{
			fprintf(stderr, "\x1b[31mSyntax error: "
				"Unexpected math operator %c (%d)\x1b[39m\n", c, (int)c);
			return -1;
		}
		else
		{
			fprintf(stderr, "\x1b[31mSyntax error: "
				"Unknown char %c (%d)\x1b[39m\n", c, (int)c);
			return -1;
		}
	}
	return 0;
	#undef TOP_PMS
}

/* Parse the given string as a complete Helv program. */
int parse_all(const char* src, sin_map_t* sinm, prog_table_t* table)
{
	unsigned int prog_index = prog_table_alloc_prog_index(table);
	table->array[prog_index] = (prog_t){0};
	pms_st_t pms_st = {0};
	*pms_st_push_init(&pms_st) = (pms_t){0};
	rs_t src_rs;
	rs_init(&src_rs, src);
	int res = parse_prog(&src_rs, prog_index, table, PROG_FILE, &pms_st, sinm);
	rs_cleanup(&src_rs);
	pms_st_cleanup(&pms_st);
	return res;
}

void prog_table_print(const prog_table_t* table)
{
	for (unsigned int i = 0; i < table->len; i++)
	{
		printf("prog %d:\n", (int)i);
		prog_t* prog = &table->array[i];
		for (unsigned int j = 0; j < prog->len; j++)
		{
			instr_t* instr = &prog->array[j];
			switch (instr->name)
			{
				case INSTR_NOP:
					printf("\tnop\n");
				break;
				case INSTR_GLOBAL_HALT:
					printf("\tglobal_halt\n");
				break;
				case INSTR_LOCAL_HALT:
					printf("\tlocal_halt\n");
				break;
				case INSTR_PUSH_IMMUINT:
					printf("\tpush_immuint %u\n", (unsigned int)instr->uint64);
				break;
				case INSTR_SWAP_ANY:
					printf("\tswap_any\n");
				break;
				case INSTR_DUPLICATE_ANY:
					printf("\tduplicate_any\n");
				break;
				case INSTR_KILL_ANY:
					printf("\tkill_any\n");
				break;
				case INSTR_ADD_UINT:
					printf("\tadd_uint\n");
				break;
				case INSTR_SUBTRACT_UINT:
					printf("\tsubtract_uint\n");
				break;
				case INSTR_MULTIPLY_UINT:
					printf("\tmultiply_uint\n");
				break;
				case INSTR_DIVIDE_UINT:
					printf("\tdivide_uint\n");
				break;
				case INSTR_MODULUS_UINT:
					printf("\tmodulus_uint\n");
				break;
				case INSTR_PRINT_UINT_AS_CHAR:
					printf("\tprint_uint_as_char\n");
				break;
				case INSTR_PRINT_UINT:
					printf("\tprint_uint\n");
				break;
				case INSTR_PRINT_STACK:
					printf("\tprint_stack\n");
				break;
				case INSTR_EXECUTE_CODEINDEX:
					printf("\texecute_codeindex\n");
				break;
				case INSTR_IFELSE_CODEINDEX:
					printf("\tif_codeindex\n");
				break;
				case INSTR_DOWHILE_CODEINDEX:
					printf("\tdowhile_codeindex\n");
				break;
			}
		}
	}
}

/* Write the C statements that do what should do the given instruction. */
void emit_c_instr(gs_t* out_gs, const instr_t* instr)
{
	#define EMIT(...) gs_append_f(out_gs, __VA_ARGS__)
	switch (instr->name)
	{
		case INSTR_NOP:
			EMIT("\t;\n"); 
		break;
		case INSTR_LOCAL_HALT:
			EMIT("\treturn;\n");
		break;
		case INSTR_GLOBAL_HALT:
			EMIT("\texit(0);\n");
		break;
		case INSTR_PUSH_IMMUINT:
			EMIT("\ts[i++] = %d;\n", (int)instr->uint64);
		break;
		case INSTR_SWAP_ANY:
			EMIT(
				"\t{\n"
				"\t\tuint64_t tmp = s[i-2];\n"
				"\t\ts[i-2] = s[i-1];\n"
				"\t\ts[i-1] = tmp;\n"
				"\t}\n"
			);
		break;
		case INSTR_DUPLICATE_ANY:
			EMIT("\ts[i] = s[i-1];\n");
			EMIT("\ti++;\n");
		break;
		case INSTR_KILL_ANY:
			EMIT("\ti--;\n");
		break;
		case INSTR_ADD_UINT:
			EMIT("\ts[i-2] = s[i-1] + s[i-2];\n");
			EMIT("\ti--;\n");
		break;
		case INSTR_SUBTRACT_UINT:
			EMIT("\ts[i-2] = s[i-1] - s[i-2];\n");
			EMIT("\ti--;\n");
		break;
		case INSTR_MULTIPLY_UINT:
			EMIT("\ts[i-2] = s[i-1] * s[i-2];\n");
			EMIT("\ti--;\n");
		break;
		case INSTR_DIVIDE_UINT:
			EMIT("\ts[i-2] = s[i-1] / s[i-2];\n");
			EMIT("\ti--;\n");
		break;
		case INSTR_MODULUS_UINT:
			EMIT("\ts[i-2] = s[i-1] %% s[i-2];\n");
			EMIT("\ti--;\n");
		break;
		case INSTR_PRINT_UINT_AS_CHAR:
			EMIT("\tprintf(\"%%c\", (unsigned int)s[--i]);\n");
		break;
		case INSTR_PRINT_UINT:
			EMIT("\tprintf(\"%%u\", (unsigned int)s[--i]);\n");
		break;
		case INSTR_PRINT_STACK:
			EMIT("\tfor (unsigned int j = 0; j < i-1; j++)\n");
			EMIT("\t{\n");
			EMIT("\t\tprintf(\"%%d \", (int)s[j]);\n");
			EMIT("\t}\n");
			EMIT("\tprintf(\"%%d\\n\", (int)s[i-1]);\n");
		break;
		case INSTR_EXECUTE_CODEINDEX:
			EMIT("\tprog_table[s[--i]]();\n");
		break;
		case INSTR_IFELSE_CODEINDEX:
			EMIT("\t{\n");
			EMIT("\t\tuint64_t condition = s[--i];\n");
			EMIT("\t\tuint64_t index_if = s[--i];\n");
			EMIT("\t\tuint64_t index_else = s[--i];\n");
			EMIT("\t\tprog_table[condition ? index_if : index_else]();\n");
			EMIT("\t}\n");
		break;
		case INSTR_DOWHILE_CODEINDEX:
			EMIT("\t{\n");
			EMIT("\t\tuint64_t index_do = s[--i];\n");
			EMIT("\t\tdo\n");
			EMIT("\t\t{\n");
			EMIT("\t\t\tprog_table[index_do]();\n");
			EMIT("\t\t} while (s[--i]);\n");
			EMIT("\t}\n");
		break;
		default:
			EMIT("\tfprintf(stderr, "
				"\"TODO: Emit the instruction %d\\n\");\n", instr->name);
		break;
	}
	#undef EMIT
}

/* Write the C statements that do what should do the given program. */
void emit_c_prog(gs_t* out_gs, const prog_t* prog)
{
	for (unsigned int i = 0; i < prog->len; i++)
	{
		emit_c_instr(out_gs, &prog->array[i]);
	}
}

/* Write the complete final ultimate valid C program. */
void emit_c_all(gs_t* out_gs, const prog_table_t* table)
{
	#define EMIT(...) gs_append_f(out_gs, __VA_ARGS__)
	EMIT("#include <stdio.h>\n");
	EMIT("#include <stdint.h>\n");
	EMIT("void exit(int);\n");
	EMIT("uint64_t s[9999];\n");
	EMIT("unsigned int i = 0;\n");
	for (unsigned int i = 0; i < table->len; i++)
	{
		EMIT("void code_%u(void);\n", i);
	}
	EMIT("void(*prog_table[])(void) = {\n");
	for (unsigned int i = 0; i < table->len-1; i++)
	{
		EMIT("\tcode_%u,\n", i);
	}
	EMIT("\tcode_%u\n", table->len-1);
	EMIT("};\n");
	for (unsigned int i = 0; i < table->len; i++)
	{
		EMIT("void code_%u(void)\n", i);
		EMIT("{\n");
		emit_c_prog(out_gs, &table->array[i]);
		EMIT("}\n");
	}
	EMIT("int main(void)\n");
	EMIT("{\n");
	EMIT("\tprog_table[0]();\n");
	EMIT("}\n");
	#undef EMIT
}

/* Returns an allocated buffer containing the file's content. */
char* read_file(const char* file_path)
{
	FILE* file = fopen(file_path, "r");
	if (file == NULL)
	{
		fprintf(stderr, "File error: failed to open \"%s\"\n", file_path);
		return NULL;
	}
	fseek(file, 0, SEEK_END);
	int len = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* buffer = xmalloc(len+1);
	fread(buffer, 1, len, file);
	buffer[len] = '\0';
	fclose(file);
	return buffer;
}

int main(int argc, char** argv)
{
	char* src_file_path = NULL;
	char* out_file_path = "out.c";
	for (unsigned int i = 1; i < (unsigned int)argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (strcmp(argv[i], "-o") == 0)
			{
				if (i == (unsigned int)argc-1)
				{
					printf("Option -o expects a file path following.\n");
					continue;
				}
				if (out_file_path != NULL)
				{
					printf("Note: Too much out files provided.\n");
				}
				out_file_path = argv[++i];
			}
			else if (strcmp(argv[i], "-d") == 0)
			{
				g_debug = 1;
			}
		}
		else
		{
			if (src_file_path != NULL)
			{
				printf("Note: Too much source files provided.\n");
			}
			src_file_path = argv[i];
		}
	}

	if (src_file_path == NULL)
	{
		printf("No source file provided.\n");
		return 0;
	}

	char* src = read_file(src_file_path);

	sin_map_t sinm = {0};
	*sin_map_alloc_sin(&sinm) = (sin_t){
		.name = INSTR_NOP,
		.str = "nop", .str3 = "nop", .one_char = 'n',
	};
	*sin_map_alloc_sin(&sinm) = (sin_t){
		.name = INSTR_GLOBAL_HALT,
		.str = "ghalt", .str3 = NULL, .one_char = '\0',
	};
	*sin_map_alloc_sin(&sinm) = (sin_t){
		.name = INSTR_LOCAL_HALT,
		.str = "halt", .str3 = "hlt", .one_char = 'h',
	};
	*sin_map_alloc_sin(&sinm) = (sin_t){
		.name = INSTR_SWAP_ANY,
		.str = "swap", .str3 = "swp", .one_char = 's',
	};
	*sin_map_alloc_sin(&sinm) = (sin_t){
		.name = INSTR_DUPLICATE_ANY,
		.str = "duplicate", .str3 = "dup", .one_char = 'd',
	};
	*sin_map_alloc_sin(&sinm) = (sin_t){
		.name = INSTR_KILL_ANY,
		.str = "kill", .str3 = "kil", .one_char = 'k',
	};
	*sin_map_alloc_sin(&sinm) = (sin_t){
		.name = INSTR_ADD_UINT,
		.str = "add", .str3 = "add", .one_char = '+',
	};
	*sin_map_alloc_sin(&sinm) = (sin_t){
		.name = INSTR_SUBTRACT_UINT,
		.str = "substract", .str3 = "sub", .one_char = '-',
	};
	*sin_map_alloc_sin(&sinm) = (sin_t){
		.name = INSTR_MULTIPLY_UINT,
		.str = "multiply", .str3 = "mul", .one_char = '*',
	};
	*sin_map_alloc_sin(&sinm) = (sin_t){
		.name = INSTR_DIVIDE_UINT,
		.str = "divide", .str3 = "div", .one_char = '/',
	};
	*sin_map_alloc_sin(&sinm) = (sin_t){
		.name = INSTR_MODULUS_UINT,
		.str = "modulus", .str3 = "mod", .one_char = '%',
	};
	*sin_map_alloc_sin(&sinm) = (sin_t){
		.name = INSTR_PRINT_UINT_AS_CHAR,
		.str = "print", .str3 = "prt", .one_char = 'p',
	};
	*sin_map_alloc_sin(&sinm) = (sin_t){
		.name = INSTR_EXECUTE_CODEINDEX,
		.str = "execute", .str3 = "exe", .one_char = 'x',
	};
	*sin_map_alloc_sin(&sinm) = (sin_t){
		.name = INSTR_IFELSE_CODEINDEX,
		.str = "ifelse", .str3 = "ife", .one_char = 'i',
	};
	*sin_map_alloc_sin(&sinm) = (sin_t){
		.name = INSTR_DOWHILE_CODEINDEX,
		.str = "dowhile", .str3 = "dwl", .one_char = 'l',
	};

	prog_table_t table = {0};

	if (parse_all(src, &sinm, &table) != 0)
	{
		return -1;
	}

	if (g_debug)
	{
		prog_table_print(&table);
	}

	gs_t out_gs;
	gs_init(&out_gs);

	emit_c_all(&out_gs, &table);

	FILE* out_file = fopen(out_file_path, "w");
	fputs(out_gs.str, out_file);
	fclose(out_file);

	gs_cleanup(&out_gs);
	return 0;
}
