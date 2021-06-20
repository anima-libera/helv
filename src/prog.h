
#ifndef HELV_PROG_HEADER
#define HELV_PROG_HEADER

#include "utils.h"
#include <stdint.h>
#include <assert.h> /* static_assert */

/* Elementary macro instruction id, can and should fit in a byte. */
enum instr_id_t
{
	INSTR_ID_NOP = 0,
	INSTR_ID_PUSH_IMM, /* Immutable byte value follows. */
	INSTR_ID_KILL,
	INSTR_ID_DUPLICATE,
	INSTR_ID_SWAP,
	INSTR_ID_GET,
	INSTR_ID_SET,
	INSTR_ID_HEIGHT,
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
	int is_finished; /* Is this program fully parsed? */
};
typedef struct prog_t prog_t;

#define ASSERT_CHECK_PROG_PTR(prog_ptr_) \
	do \
	{ \
		ASSERT(prog_ptr_ != NULL, "The pointer is NULL\n"); \
		ASSERT_CHECK_DARRAY(prog_ptr_->len, prog_ptr_->cap, \
			prog_ptr_->array); \
	} while (0)

void prog_cleanup(prog_t* prog);

/* Extends the program by len uninitialized bytes,
 * and returns a pointer to the newly added bytes that must all be used. */
uint8_t* prog_alloc(prog_t* prog, unsigned int len);

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

void full_prog_cleanup(full_prog_t* full_prog);

/* Adds an empty program to the given full program,
 * and returns the new program's index. */
unsigned int full_prog_alloc_index(full_prog_t* full_prog);

#endif /* HELV_PROG_HEADER */
