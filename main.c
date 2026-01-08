#include "include/xmalloc.h"

static int *get_int_ptr(void)
{
	int *int_ptr = xmalloc(sizeof(int));
	return int_ptr;
}

static void nop(void)
{
}
/* TEST IT, JOIN BLOKCS AFTER FREEING BIG BLOCKS SO SPLITTING AND REUSING CAN BE DONE 
 * ALSO CHECK FOR TAIL POINTER IN SPLITTING
 *
 * OR PERHAPS WHEN A SIZE-X BLOCK IS BEING REQUESTED AND MAKE IT RECURSIVE FOR EVERY
 * NEXT BLOCK THAT IS FREE.
 */
int main(void)
{
	int *my_big_ptr = xmalloc(100);
	nop();

	if (!my_big_ptr)
		return 1;

	xfree(my_big_ptr);
	nop();

	int *my_new_ptr = get_int_ptr();
	
	if (!my_new_ptr)
		return 1;

	nop();

	char *new_char_ptr = xmalloc(sizeof(char));

	if (!new_char_ptr)
		return 1;
	
	nop();


	return 0;
}
