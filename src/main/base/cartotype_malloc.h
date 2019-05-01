/*
CARTOTYPE_MALLOC.H
Copyright (C) 2004 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_MALLOC_H__
#define CARTOTYPE_MALLOC_H__

#ifdef __cplusplus
extern "C" 
{
#endif

#define FIRST_BLOCK_SIZE (900 * 1024)
#define MINIMUM_BLOCK_SIZE (64 * 1024)
#define MINIMUM_BLOCK_SIZE_SMALL_HEAP (16 * 1024)

/** 
Initialize the sbrk emulation. The sbrk emulator allocates memory blocks
for use within the dlmalloc allocator. Every time dlmalloc allocates
a new block, this block has a minimum size which is the second
argument of this function. The first argument is the minimum size of
the first allocated block, for efficiency reasons this should be a 
large block if the application is expected to use a lot of memory. It should not
be more than the peak amount of memory the application is expected to use
as this will waste memory. This function, if called, must be called before any memory
is allocated within CartoType.
If it is not called it will use the figures defined in this header file. 
The default for a 'small heap' is used if the size of the first block is
more than half the actual heap size.
*/
void sbrk_init(long aMinimumFirstBlockSize,long aMinimumBlockSize);

/** 
Release the heap memory used by the sbrk emulation. This function must be
called after all CartoType memory has been released, just before the application
terminates.
 */
void sbrk_shutdown();

#ifdef __cplusplus
}
#endif

#endif
