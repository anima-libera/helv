
#ifndef HELV_UTILS_HEADER
#define HELV_UTILS_HEADER

#include <stdlib.h>
#include <stdio.h>

#ifdef __GNUC__
	#define ATTRIBUTE(...) __attribute__ ((__VA_ARGS__))
#else
	#define ATTRIBUTE(...)
#endif

#define xmalloc malloc
#define xcalloc calloc
#define xrealloc realloc
#define xfree free

#ifdef DEBUG
	#define ENABLE_ASSERT
#endif

#ifdef ENABLE_ASSERT
	/* Not using the standard assert macro just so that we really control if
	 * assertions are optimized out or not.
	 * After the condition, printf-like arguments are requiered.
	 * It is ok for assertions to be expensive, as long as they may catch a bug
	 * early, they are debug-build-only anyway.
	 * It is ok for assertions to be redundant, let's abuse assertions,
	 * lets gooo, C memory safe language confirmed! */
	#define ASSERT(condition_, ...) \
		do \
		{ \
			if (!(condition_)) \
			{ \
				fprintf(stderr, "Assertion failed: " \
					"At line %d in function %s in file " __FILE__ "\n", \
					__LINE__, __func__); \
				fprintf(stderr, "The condition that is false is %s\n", \
					#condition_); \
				fprintf(stderr, __VA_ARGS__); \
				exit(EXIT_FAILURE); \
			} \
		} while (0)

	/* The given statement (think carefully about where to put the semicolons)
	 * is executed only if assertions are enabled. */
	#define CODE_FOR_ASSERT(statement_) statement_
#else
	#define ASSERT(condition_, ...) do { } while (0)
	#define CODE_FOR_ASSERT(statement_)
#endif

unsigned int umax(unsigned int a, unsigned int b);

#define DARRAY_RESIZE_IF_NEEDED(len_, cap_, ptr_, elem_type_) \
	do \
	{ \
		if (len_ > cap_) \
		{ \
			unsigned int new_cap_wanted_ = umax(len_, \
				((float)cap_ + 2.3f) * 1.6f); \
			elem_type_* new_array_ = xrealloc(ptr_, \
				new_cap_wanted_ * sizeof(elem_type_)); \
			ASSERT(new_array_ != NULL, "Reallocation failed"); \
			ptr_ = new_array_; \
			cap_ = new_cap_wanted_; \
		} \
	} while (0)

#define ASSERT_CHECK_DARRAY(len_, cap_, ptr_) \
	do \
	{ \
		ASSERT(len_ <= cap_, \
			"The length (%u) is greater than the capacity (%u)\n", \
			len_, cap_); \
		ASSERT(ptr_ != NULL || cap_ == 0, \
			"The pointer is null but the capacity (%u) is not\n", cap_); \
		ASSERT(cap_ != 0 || ptr_ == NULL, \
			"The capacity is 0 but the pointer is non-null\n"); \
	} while (0)

/* Returns an allocated buffer containing the file's content. */
char* read_file(const char* file_path);

#endif /* HELV_UTILS_HEADER */
