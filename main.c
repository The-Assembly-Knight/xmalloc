#include "include/xmalloc.h"

static int *get_int_ptr(void)
{
	int *int_ptr = xmalloc(sizeof(int));
	return int_ptr;
}

static char *get_char_ptr(void)
{
	char *char_ptr = xmalloc(sizeof(char));
	return char_ptr;
}

static void nop(void)
{
}

int main(void)
{
	char *my_char_ptr = get_char_ptr();
	int *my_int_ptr = get_int_ptr();

	if (!my_char_ptr || !my_int_ptr)
		return 1;

	nop();

	return 0;
}
