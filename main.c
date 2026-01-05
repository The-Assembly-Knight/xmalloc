#include "include/xmalloc.h"

static int *get_int_ptr(void)
{
	int *int_ptr = xmalloc(sizeof(int));
	return int_ptr;
}

int main(void)
{
	int *int_ptr = get_int_ptr();
	if (!int_ptr)
		return 1;
	return 0;
}
