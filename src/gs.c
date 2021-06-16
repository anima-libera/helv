
#include "gs.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h> /* strlen */
#include <stdarg.h>

void gs_init(gs_t* gs)
{
	/* Making sure that str is a valid C string even if empty. */
	gs->str = xmalloc(1);
	gs->str[0] = '\0';
	gs->len = 1;
	gs->cap = 1;
}

void gs_cleanup(gs_t* gs)
{
	ASSERT_CHECK_GS_PTR(gs);
	free(gs->str);
}

void gs_append_f(gs_t* gs, const char* format, ...)
{
	ASSERT_CHECK_GS_PTR(gs);
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
		DARRAY_RESIZE_IF_NEEDED(gs->len, gs->cap, gs->str, char);
		gs->len -= requested_len;
		vsprintf(&gs->str[gs->len-1], format, ap_2);
	}
	va_end(ap_2);
	gs->len += requested_len;
}
