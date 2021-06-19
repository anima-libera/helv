
#include "utils.h"
#include "gs.h"
#include "prog.h"
#include "interpreter.h"
#include "emit_c.h"
#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h> /* strcmp */

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
