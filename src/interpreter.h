
#ifndef HELV_INTERPRETER_HEADER
#define HELV_INTERPRETER_HEADER

#include "utils.h"
#include "prog.h"
#include <stdint.h>

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

void st_cleanup(st_t* st);
void st_push(st_t* st, uint8_t byte);
uint8_t st_pop(st_t* st);

void execute_full_prog(const full_prog_t* full_prog, st_t* st);

#endif /* HELV_INTERPRETER_HEADER */
