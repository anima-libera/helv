
#ifndef HELV_GS_HEADER
#define HELV_GS_HEADER

#include "utils.h"
#include <string.h> /* strlen */

/* Growable string. */
struct gs_t
{
	unsigned int len; /* Counting the null terminator in. */
	unsigned int cap;
	char* str; /* Always valid C string (between init and cleanup). */
};
typedef struct gs_t gs_t;

#define ASSERT_CHECK_GS_PTR(gs_ptr_) \
	do \
	{ \
		ASSERT(gs_ptr_ != NULL, "The pointer is NULL\n"); \
		ASSERT(gs_ptr_->str != NULL, "The char pointer is NULL\n"); \
		ASSERT(gs_ptr_->len <= gs_ptr_->cap, \
			"The length (%u) is greater than the capacity (%u)\n", \
			gs_ptr_->len, gs_ptr_->cap); \
		CODE_FOR_ASSERT(unsigned int strlen_result_inc = \
			strlen(gs_ptr_->str) + 1); \
		ASSERT(strlen_result_inc == gs_ptr_->len, \
			"The length \"len\" stored in the growable string (%u) " \
			"doesn't equal the result of strlen(str)+1 (%u) as it should\n", \
			gs_ptr_->len, strlen_result_inc); \
	} while (0)

void gs_init(gs_t* gs);

void gs_cleanup(gs_t* gs);

/* Appends the printf-formatted arguments to the given growable string. */
void gs_append_f(gs_t* gs, const char* format, ...)
	ATTRIBUTE(format (printf, 2, 3));

#endif /* HELV_GS_HEADER */
