#pragma once

#include <stddef.h>

void *xmalloc(const size_t size);
void xfree(const void *ptr);
