#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "../include/handle_error.h"
#include "../include/xmalloc.h"

#define MIN_ALLOCATION_SIZE 1
#define MAX_ALLOCATION_SIZE INTPTR_MAX
#define DEFAULT_ALIGNMENT 8

/* memory block structure
 * 
 * @mem free - true if block has not been occupied, otherwise false.
 * @mem size - total size of the block (including alignment)
 * @mem size_left - unused memory within the memory block
 * @mem alignment - alignment used to allocate the memory after mblock struct
 * @mem *next - next memory block in the linked list
 *
 */
struct mblock {
	bool free;
	size_t size;
	size_t size_left;
	size_t alignment;
	struct mblock *next;
};

/* head and tail of memory blocks linked list */
static struct mblock *head = NULL;
static struct mblock **tail = &head;

/* Finds and returns a block of memory with equal or more memory left than
 * @param size.
 * 
 * Description: goes through the linked list of memory blocks already allocated
 * to find a block with enough available space for the size requested.
 *
 * Parameters:
 * @param size - minimum size left required from a block.
 *
 * Returns: pointer to the block of memory with enough space left.
 *
 * Notes: if no block of memory with enough space is found then returns NULL.
 *
 */
static struct mblock *get_from_list(const size_t size)
{
	struct mblock *current = head;
	while (current) {
		if (current->size_left >= size)
			return current;
		current = current->next;
	}
	return NULL;
}

static void init_block(struct mblock *mblock, const bool free, const size_t size,
			const size_t size_left, const size_t alignment,
			struct mblock *next)
{
	mblock->free = free;
	mblock->size = size;
	mblock->size_left = size_left;
	mblock->alignment = alignment;
	mblock->next = next;
}

/* Initializes @param mblock as a new block and sets its members based on
 * the other parameters.
 *
 * Description: initializes mblock by setting all of its members taking into
 * account @param size.
 *
 * Parameters:
 * @param size - value to which @mem size must be initialized to.
 *
 * Returns: Nothing
 *
 * Notes: ** IMPORTANT ** TODO
 * Some of the parameters have not been set correctly because there are other
 * things yet to implement (things that are neccessary for those parameters),
 * thus @mem size_left and @mem alignment and @mem next are intentionately
 * wrong.
 *
 *TODO: SUBSTITUTE BODY BY INIT_BLOCK FUNC WITH SOME SPECIFIC PARAMS
 */
static void occupy_block(struct mblock *mblock, const size_t size,
			const size_t size_occupied, struct mblock *next)
{

	mblock->free = false;
	mblock->size = size;
	mblock->size_left = size - size_occupied;
	mblock->alignment = 0;
	mblock->next = next;
}


/* Splits a memory block into two (not necessarily equal to each other)
 *
 * Description: Divides size of @param mblock into two blocks: one just enough
 * to allocate what was already allocated in @param mblock while the other one
 * contains the rest of the space (including space for another mblock struct)
 *
 * Parameters:
 * @param mblock - original block of memory that will be split into two
 * @param size - space required by the new allocation from the second block
 *
 * Returns: Nothing
 *
 * TODO MAKE FUNCTION PARAMETER NAME EITHER MBLOCK OR BLOCK NOT BOTH FOR
 * CONSISTENCY
 * */

static void split_block(struct mblock *mblock, const size_t size)
{
	if (size > mblock->size_left) {
		handle_error("size to split block is greater than the size_left\
				of the block", SEVERITY_FATAL);
	}

	mblock->size = mblock->size - mblock->size_left;

	struct mblock *new_block = (struct mblock *)((char *)mblock + sizeof(*mblock) + mblock->size);
	init_block(new_block, true, mblock->size_left, mblock->size_left, 0, mblock->next);
	mblock->next = new_block;
	mblock->size_left = 0;
}

/* Allocates memory in the heap for a new memory block of size @param size
 *
 * Description: By using sbrk, it allocates memory in the heap based on
 * @param size.
 *
 * Parameters:
 * @param size - size of the allocation (without including the size of the
 * mblock structure itself)
 *
 * Returns: A pointer to the mblock responsible for the memory allocated.
 *
 * Notes: In case the allocation can not be performed or an error ocurred while
 * performing it, the function will alert the user through a warning and return
 * a NULL pointer.
 *
 */
static struct mblock *allocate_block(const size_t size)
{
	if (size > MAX_ALLOCATION_SIZE) {
		handle_error("allocation size is over MAX_ALLOCATION_SIZE",
				SEVERITY_WARNING);
		return NULL;
	}

	void *new_block = sbrk((intptr_t)size);
	if (new_block == (void *) - 1) {
		handle_error("sbrk could not allocate memory for a new block",
				SEVERITY_WARNING);
		return NULL;
	}
	return (struct mblock *)new_block;
}

/* Adds a new memory block to the end of the list
 *
 * Description: Adds a new memory block node @param mblock to the end of the
 * linked list
 *
 * Parameters:
 * @param mblock - new memory block to be added to the end of the linked list.
 *
 * Returns: Nothing
 *
 * Notes: It is taking as granted that @param mblock is not NULL otherwise
 * UB is likely to be the result of calling the function.
 *
 */
static void add_to_list(struct mblock *block)
{
	if (!head) {
		head = block;
	}
	else {
		(*tail)->next = block;
		tail = &(*tail)->next;
	}
}

/* Gets the value of @param size if it were aligned to @param alignment.
 *
 * Parameters:
 * @param size - original value of size
 * @param alignment - alignment to be applied to size
 *
 * Returns: size after alignment being applied to it
 *
 */
static size_t get_size_aligned(const size_t size, const size_t alignment)
{
	if (size % alignment == 0)
		return size;

	if (size <= alignment)
		return alignment;

	return (size + alignment - (size % alignment));
}

/* Checks if @param block is part of blocks' linked list*
 *
 * Description: Checks if @param block point to a node of the blocks' linked
 * list by checking if both addresses are the same.
 *
 * Parameters:
 * @param block - pointer to block of memory that must be checked if it is
 * part of the list
 *
 * Returns: true if the @param block pointer is part of the linked list,
 * false otherwise
 *
 */
static bool is_in_list(const struct mblock *block)
{
	struct mblock *current = head;
	while (current) {
		if (current == block)
			return true;
		current = current->next;
	}
	return false;
}

/* Allocates memory in heap and returns a pointer to the allocation.
 *
 * Description: allocates memory through sbrk based on @param size if there is
 * no enough already-allocated memory in the lists.
 *
 * Parameters:
 * @param size - size of the allocation
 *
 * Returns: pointer to memory allocated
 *
 * Notes: If the size of the allocation is greater than the maximum allocation
 * size (INTPTR_MAX) or less than the minimum allocation size (1)
 * 	
 */
void *xmalloc(const size_t size)
{
	if (size < MIN_ALLOCATION_SIZE) { 		
		handle_error("size passed to xmalloc is under MIN_ALLOCATION_SIZE",
				SEVERITY_WARNING);
		return NULL;
	}

	const size_t size_aligned = get_size_aligned(size, DEFAULT_ALIGNMENT);
	size_t alloc_size = size_aligned;

	if (alloc_size + sizeof(struct mblock) <= SIZE_MAX) {
		alloc_size += sizeof(struct mblock);
	} else {
		handle_error("alloc_size > SIZE_MAX", SEVERITY_FATAL);
	}

	struct mblock *available_block = get_from_list(size_aligned);
	if (available_block) {
		if (!available_block->free && available_block->size_left > alloc_size) {
			split_block(available_block, alloc_size);
			available_block = available_block->next;
		}
		occupy_block(available_block, available_block->size,
				size_aligned, available_block->next);
		return (void *)(available_block + 1);
	}

	struct mblock *new_block = allocate_block(alloc_size);
	occupy_block(new_block, size_aligned, size_aligned, NULL);
	add_to_list(new_block);
	return (void *)(new_block + 1);
}

/* Frees an allocation made by xmalloc
 *
 * Description: Makes sure the @param ptr passed was memory allocated by xmalloc
 * and frees that block of memory making it available for reuse.
 *
 * Parameters:
 * @param ptr - pointer to the beginning of the memory managed by an mblock
 * structure.
 *
 * Returns: Nothing
 *
 * Notes: This function makes sure that ptr is not NULL nor 
 *
 */
void xfree(const void *ptr)
{
	if (!ptr) {
		handle_error("ptr passed to xfree is NULL", SEVERITY_WARNING);
		return;
	}

	struct mblock *adj_mblock = (struct mblock*)ptr - 1;

	if (!is_in_list(adj_mblock)) {
		handle_error("ptr passed to xfree was not allocated by xmalloc",
				SEVERITY_WARNING);
		return;
	}

	adj_mblock->free = true;
	adj_mblock->size_left = adj_mblock->size;
}
