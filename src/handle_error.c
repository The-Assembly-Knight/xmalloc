#include <stdlib.h>
#include <stdio.h>

#include "../include/handle_error.h"

static void handle_warning(const char *message)
{
	fprintf(stderr,"WARNING: %s\n", message);
}

static void handle_fatal(const char *message)
{
	fprintf(stderr, "FATAL: %s\n", message);
	abort();
}

void handle_error(const char *message, const enum severity_type severity)
{
	if (!message)
		handle_error("message passed to handle_error is NULL", SEVERITY_FATAL);

	if (severity == SEVERITY_WARNING)
		handle_warning(message);
	else
		handle_fatal(message);
}
