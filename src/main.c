
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
	"Assertion failed: At %d in function %s in file " __FILE__ "\n", \
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
	INSTR_DUP_ANY,
	INSTR_KILL_ANY,
	INSTR_ADD_UINT,
	INSTR_SUB_UINT,
	INSTR_MUL_UINT,
	INSTR_DIV_UINT,
	INSTR_MOD_UINT,
	INSTR_PRINT_UINT_AS_CHAR,
	INSTR_PRINT_UINT,
	INSTR_PRINT_STACK,
	INSTR_PUSH_IMMCODEINDEX,
	INSTR_EXECUTE_CODEINDEX,
	INSTR_IF_CODEINDEX,
	INSTR_DOWHILE_CODEINDEX,
};
typedef enum instr_name_t instr_name_t;

struct instr_t
{
	instr_name_t name;
	union
	{
		uint64_t uint64;
		char* string;
		unsigned int code_index;
		char* def_name;
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
struct growstr_t
{
	unsigned int len; /* Counts in the null terminator */
	unsigned int cap;
	char* str;
};
typedef struct growstr_t growstr_t;

void growstr_init(growstr_t* gs)
{
	gs->str = malloc(1);
	gs->str[0] = '\0';
	gs->len = 1;
	gs->cap = 1;
}

void growstr_cleanup(growstr_t* gs)
{
	free(gs->str);
}

/* Appends the printf-formatted arguments to the given growable string. */
void growstr_append_f(growstr_t* gs, const char* format, ...)
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
	ARRAY_RESIZE_IF_NEEDED(table->len, table->cap,
		table->array, prog_t);
	return table->len-1;
}

enum prog_type_t
{
	PROG_FILE,
	PROG_BRACKETS,
};
typedef enum prog_type_t prog_type_t;

/* Parse the given string as a program, such as the content of a [ ] block.
 * Returns non-zero on error.
 * Sets the parsed_len parameter to the number of chars actually parsed,
 * but only if parsed_len is non-null. */
int parse(const char* src, unsigned int prog_index, prog_table_t* table,
	unsigned int* parsed_len, prog_type_t prog_type)
{
	unsigned int i = 0;
	while (1)
	{
		char c = src[i];
		if (c == '[')
		{
			unsigned int sub_prog_index = prog_table_alloc_prog_index(table);
			table->array[sub_prog_index] = (prog_t){0};
			unsigned int parsed_len;
			if (parse(&src[i+1],
				sub_prog_index, table, &parsed_len, PROG_BRACKETS) != 0)
			{
				return -1;
			}
			i += 1 + parsed_len;
			if (src[i] == ']')
			{
				i++;
			}
			*prog_alloc_instr(&table->array[prog_index]) = (instr_t){
				.name = INSTR_PUSH_IMMCODEINDEX, .code_index = sub_prog_index
			};
		}
		else if ('0' <= c && c <= '9')
		{
			unsigned int j;
			unsigned int constant_value = 0;
			for (j = i; '0' <= src[j] && src[j] <= '9'; j++)
			{
				constant_value *= 10;
				constant_value += src[j] - '0';
			}
			*prog_alloc_instr(&table->array[prog_index]) = (instr_t){
				.name = INSTR_PUSH_IMMUINT, .uint64 = constant_value
			};
			i = j;
		}
		else if ('a' <= c && c <= 'z')
		{
			instr_t* instr = prog_alloc_instr(&table->array[prog_index]);
			*instr = (instr_t){0};
			switch (c)
			{
				case 's':
					instr->name = INSTR_SWAP_ANY;
				break;
				case 'd':
					instr->name = INSTR_DUP_ANY;
				break;
				case 'k':
					instr->name = INSTR_KILL_ANY;
				break;
				case 'm':
					switch (src[i+1])
					{
						case '+':
							instr->name = INSTR_ADD_UINT;
						break;
						case '-':
							instr->name = INSTR_SUB_UINT;
						break;
						case '*':
							instr->name = INSTR_MUL_UINT;
						break;
						case '/':
							instr->name = INSTR_DIV_UINT;
						break;
						case '%':
							instr->name = INSTR_MOD_UINT;
						break;
						default:
							fprintf(stderr, "\x1b[31mSyntax error: "
								"Unknown math instr %c (%d)\x1b[39m\n",
								src[i+1], (int)src[i+1]);
						break;
					}
					i++;
				break;
				case 'p':
					instr->name = INSTR_PRINT_UINT_AS_CHAR;
				break;
				case 'u':
					instr->name = INSTR_PRINT_UINT;
				break;
				case 'a':
					instr->name = INSTR_PRINT_STACK;
				break;
				case 'x':
					instr->name = INSTR_EXECUTE_CODEINDEX;
				break;
				case 'i':
					instr->name = INSTR_IF_CODEINDEX;
				break;
				case 'l':
					instr->name = INSTR_DOWHILE_CODEINDEX;
				break;
				case 'n':
					instr->name = INSTR_NOP;
				break;
				case 'h':
					instr->name = src[i+1] == 'h' ?
						INSTR_GLOBAL_HALT : INSTR_LOCAL_HALT;
				break;
				default:
					fprintf(stderr, "\x1b[31mSyntax error: "
						"Unknown instr %c (%d)\x1b[39m\n", c, (int)c);
					return -1;
				break;
			}
			i++;
		}
		else if (c == ' ' || c == '\n' || c == '\t')
		{
			i++;
		}
		else if (c == '#')
		{
			i++;
			while (src[i] != '#' && src[i] != '\0')
			{
				i++;
			}
			if (src[i] == '\0')
			{
				fprintf(stderr, "Syntax warning: "
					"Non terminated comment\n");
			}
			else
			{
				i++;
			}
		}
		else if (c == '\0' || c == ']')
		{
			if (prog_type == PROG_BRACKETS && c != ']')
			{
				fprintf(stderr, "Syntax warning: Non closed brackets\n");
			}
			else if (prog_type == PROG_FILE && c == ']')
			{
				fprintf(stderr, "Syntax warning: Too much closed brackets\n");
			}
			break;
		}
		else
		{
			fprintf(stderr, "\x1b[31mSyntax error: "
				"Unknown char %c (%d)\x1b[39m\n", c, (int)c);
			return -1;
		}
	}
	if (parsed_len != NULL)
	{
		*parsed_len = i;
	}
	return 0;
}

/* Parse the given string as a complete Helv program. */
int parse_all(const char* src, prog_table_t* table)
{
	unsigned int prog_index = prog_table_alloc_prog_index(table);
	table->array[prog_index] = (prog_t){0};
	return parse(src, prog_index, table, NULL, PROG_FILE);
}

/* Write the C statements that do what is should do the given instruction. */
void emit_c_instr(growstr_t* out_gs, const instr_t* instr)
{
	#define EMIT(...) growstr_append_f(out_gs, __VA_ARGS__)
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
		case INSTR_DUP_ANY:
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
		case INSTR_SUB_UINT:
			EMIT("\ts[i-2] = s[i-1] - s[i-2];\n");
			EMIT("\ti--;\n");
		break;
		case INSTR_MUL_UINT:
			EMIT("\ts[i-2] = s[i-1] * s[i-2];\n");
			EMIT("\ti--;\n");
		break;
		case INSTR_DIV_UINT:
			EMIT("\ts[i-2] = s[i-1] / s[i-2];\n");
			EMIT("\ti--;\n");
		break;
		case INSTR_MOD_UINT:
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
		case INSTR_PUSH_IMMCODEINDEX:
			EMIT("\ts[i++] = %d;\n", instr->code_index);
		break;
		case INSTR_EXECUTE_CODEINDEX:
			EMIT("\tprog_table[s[--i]]();\n");
		break;
		case INSTR_IF_CODEINDEX:
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
void emit_c_prog(growstr_t* out_gs, const prog_t* prog)
{
	for (unsigned int i = 0; i < prog->len; i++)
	{
		emit_c_instr(out_gs, &prog->array[i]);
	}
}

/* Write the complete final ultimate valid C program. */
void emit_c_all(growstr_t* out_gs, const prog_table_t* table)
{
	#define EMIT(...) growstr_append_f(out_gs, __VA_ARGS__)
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

	prog_table_t table = {0};

	if (parse_all(src, &table) != 0)
	{
		return -1;
	}

	growstr_t out_gs;
	growstr_init(&out_gs);

	emit_c_all(&out_gs, &table);

	FILE* out_file = fopen(out_file_path, "w");
	fputs(out_gs.str, out_file);
	fclose(out_file);

	growstr_cleanup(&out_gs);
	return 0;
}
