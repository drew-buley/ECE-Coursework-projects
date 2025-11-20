/* mem.c
 * Drew Buley
 * C20407096
 * Masterarcher300
 * Lab4: Dynamic Memory Allocation
 * ECE 2230, Fall 2024
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>

#include "mem.h"

// Global variables required in mem.c only
static int Coalescing = FALSE;      // default FALSE. TRUE if memory returned to free list is coalesced 
static int SearchPolicy = FIRST_FIT;   // default FIRST_FIT.  Can change to BEST_FIT
// NEVER use DummyChunk in your allocation or free functions!!
static mem_chunk_t DummyChunk = {&DummyChunk, 0};
static mem_chunk_t *Rover = &DummyChunk;   // one time initialization

static int NumPages = 0;
static int NumSbrkCalls = 0;

// private function prototypes
void mem_validate(void);

/* function to request 1 or more pages from the operating system.
 *
 * new_bytes must be the number of bytes that are being requested from
 *           the OS with the sbrk command.  It must be an integer 
 *           multiple of the PAGESIZE
 *
 * returns a pointer to the new memory location.  If the request for
 * new memory fails this function simply returns NULL, and assumes some
 * calling function will handle the error condition.  Since the error
 * condition is catastrophic, nothing can be done but to terminate 
 * the program.
 */
mem_chunk_t *morecore(int new_bytes) 
{
    char *cp;
    mem_chunk_t *new_p;
    // preconditions that must be true for all designs
    assert(new_bytes % PAGESIZE == 0 && new_bytes > 0);
    assert(PAGESIZE % sizeof(mem_chunk_t) == 0);
    cp = sbrk(new_bytes);
    if (cp == (char *) -1)  /* no space available */
        return NULL;
    new_p = (mem_chunk_t *) cp;
    // Code to count the number of calls to sbrk, and the number of 
    // pages that have been requested
    NumSbrkCalls++; 
    NumPages += new_bytes/PAGESIZE;
    return new_p;
}
/* A function to change default operation of dynamic memory manager.
 * This function should be called before the first call to Mem_alloc.
 *
 * search_type: sets Search Policy to BEST_FIT (defults to FIRST_FIT)
 *
 * coalescing_state: sets Coalescing to TRUE (defaults to FALSE)
 */
void Mem_configure(int coalescing_state, int search_type)
{
    assert(coalescing_state == TRUE || coalescing_state == FALSE);
    assert(search_type == FIRST_FIT || search_type == BEST_FIT);
    Coalescing = coalescing_state;
    SearchPolicy = search_type;
}

/* deallocates the space pointed to by return_ptr; it does nothing if
 * return_ptr is NULL.  
 *
 * This function assumes that the rover pointer has already been 
 * initialized and points to some memory block in the free list.
 */
void Mem_free(void *return_ptr)
{
    // precondition
    assert(Rover != NULL && Rover->next != NULL);
    
    if (return_ptr == NULL) {
        return;
    }

    //retrieving header
    mem_chunk_t *return_header = (mem_chunk_t *)return_ptr;
    return_header = return_header - 1;
    return_header->next = NULL;

    // two cases: if coalescing or not
    if (Coalescing == FALSE) {
        // insert after rover
        return_header->next = Rover->next;
        Rover->next = return_header;
    } else {
        // step 1: loop to find before and after location
        mem_chunk_t *before = NULL; //used to track if coalesced
        mem_chunk_t *after = NULL; //used to track if coalesced
        mem_chunk_t *start = Rover;
        mem_chunk_t *previous = NULL;

        do {
            previous = Rover;
            Rover = Rover->next;
            if (Rover + Rover->size_units == return_header) { //test for before to coalesce
                before = Rover;
                //insert return_header
                before->size_units += return_header->size_units;
                //if already attached return_header partialy
                if (return_header->next != NULL) {
                    before->next = return_header->next;
                    return_header->next = NULL;
                }
                return_header = before;
            }
            if (return_header + return_header->size_units == Rover) { //test for after
                
                //guard against edge case where return header block is exactly before dummy chunk
                //dummy chunk should never be coalessed!
                if (Rover->size_units == 0) {
                    continue;
                }
                //insert return_header
                return_header->next = Rover->next;
                Rover->next = NULL;
                
                //prevent edge case where list only contains dummy and previous
                if (previous != return_header) {
                    previous->next = return_header;
                }

                //adjust size to include merged block
                return_header->size_units += Rover->size_units;

                //correct placement of loop endpoint
                if (start == Rover) { //endpoint of loop was contained in merged block
                    start = return_header;
                }

                Rover = return_header;
                after = Rover; //set that we attached a block after
            }

            if ((before != NULL) && (after != NULL)) { //found both
                return;
            }

        } while (Rover != start);
        //only run if neither before or after was found
        if ((before == NULL) && (after == NULL)) {
            //need to insert in sorted manner
            start = Rover;
            do {
                Rover = Rover->next;
                if ((Rover < return_header) && (return_header < Rover->next)) { //insert in middle of chain
                    return_header->next = Rover->next;
                    Rover->next = return_header;
                    Rover = return_header;
                    break;
                }

                if (Rover > Rover->next) { //insert at wrap point of list
                    if ((return_header < Rover->next) || (return_header > Rover)) {
                        return_header->next = Rover->next;
                        Rover->next = return_header;
                        Rover = return_header;
                        break;
                    }
                }

                if (Rover == Rover->next) {//dummy block is only block in list
                    return_header->next = Rover;
                    Rover->next = return_header;
                    Rover = return_header;
                    break;
                }
                
            } while (Rover != start);
        }
    }
}

/* returns a pointer to space for an object of size nbytes, or NULL if the
 * request cannot be satisfied.  The memory is uninitialized.
 *
 * This function assumes that there is a Rover pointer that points to
 * some item in the free list.  
 */
void *Mem_alloc(const int nbytes)
{
    // precondition
    assert(nbytes > 0);
    assert(Rover != NULL && Rover->next != NULL);
    int usize = sizeof(mem_chunk_t);

    //return pointer
    mem_chunk_t *p;

    // convert requested bytes to units
    int nunits = nbytes / usize;
    if ((nbytes % usize) != 0) {
        nunits++;
    }
    nunits++; //space for header

    // search current free list for block with enough units
    mem_chunk_t *start = Rover;
    mem_chunk_t *best = NULL;
    int best_size = INT_MAX;
    mem_chunk_t *previous = NULL;
    do {
        previous = Rover;
        Rover = Rover->next;

        //used to keep track of where to alloc best fit from
        if ((SearchPolicy == BEST_FIT) && (Rover->size_units > nunits) && (best_size > Rover->size_units)) {
            //update best to be rover's location
            best = Rover;
            best_size = Rover->size_units;
        }
        if (Rover->size_units == nunits) { //exact fit so ideal case for both search policies
            p = Rover;
            //assert((p->size_units-1)*usize == nbytes);
            //assert((p->size_units-1)*usize < nbytes + usize);
            //these asserts don't work, Prof. Russel advised commenting out

            //remove header p from list
            previous->next = p->next;

            //initialize header and return userspace
            //size already correct if initial condition met
            Rover = Rover->next; //set rover position correctly after an alloc
            p->next = NULL;
            assert(p->next == NULL);
            return (void *)(p+1);

        } else if ((Rover->size_units > nunits) && (SearchPolicy == FIRST_FIT)) { //splice block
            //splice nunits from end of Rover block
            p = Rover + Rover->size_units - nunits;
            Rover->size_units -= nunits;

            //initialize header and return userspace
            p->size_units = nunits;
            p->next = NULL;
            assert((p->size_units-1)*usize < nbytes + usize);
            return (void *)(p+1);
        }
    } while (Rover != start);

    if (SearchPolicy == BEST_FIT && best_size != INT_MAX) { //checked all blocks for best option
        //splice nunits from end of best block
        p = best + best->size_units - nunits;
        best->size_units -= nunits;

        //initialize header and return userspace
        p->size_units = nunits;
        p->next = NULL;
        assert((p->size_units-1)*usize < nbytes + usize);
        return (void *)(p+1);
    }

    //request bigger block of memory as none big enough in list
    int more_bytes = PAGESIZE;
    while ((nunits*usize) > more_bytes) {
        more_bytes += PAGESIZE;
    }
    //request new page(s) of space
    mem_chunk_t *temp = morecore(more_bytes);

    if (temp == NULL) { //allocation failed
        return NULL;
    }
    //initialize temp
    temp->size_units = more_bytes/usize;
    temp->next = NULL;

    if (temp->size_units == nunits) { //alloc exactly one page
        assert((temp->size_units-1)*usize < nbytes + usize);
        return (void *)(temp+1);
    }

    //splice nunits from end of temp block
    p = temp + temp->size_units - nunits;
    temp->size_units -= nunits;

    if (Coalescing == TRUE) {
        Mem_free(temp+1); //place remainder into list in sorted position
    } else {
        temp->next = Rover->next; //insert after Rover
        Rover->next = temp;
    }

    //initialize new header and return userspace
    p->size_units = nunits;
    p->next = NULL;
    assert((p->size_units-1)*usize < nbytes + usize);
    return (void *)(p+1);
}

/* prints stats about the current free list
 *
 * -- number of items in the linked list including dummy item
 * -- min, max, and average size of each item (in bytes) but not considering
 *    dummy because its size is zero
 * -- total memory in free list (in bytes)
 * -- number of calls to sbrk and number of pages requested
 *
 * A message is printed if all the memory is in the free list
 */
void Mem_stats(void)
{
    int items_in_free_list = 0;   // count how many blocks are in the free list including dummy
    int bytes_in_free_list = 0;
    double average_block_size = 0; // do not include dummy for avg/min/max
    int min_block_size = INT_MAX;
    int max_block_size = 0;

    mem_chunk_t *start = Rover;
    do {
        //calculate stats
        Rover = Rover->next;

        items_in_free_list++;
        bytes_in_free_list += (Rover->size_units)*sizeof(mem_chunk_t);
        
        if (Rover->size_units == 0) { //DummyBlock found
            continue;
        }
        if (Rover->size_units < min_block_size) {
            min_block_size = Rover->size_units;
        }
        if (Rover->size_units > max_block_size) {
            max_block_size = Rover->size_units;
        }
    } while(Rover != start);

    if (items_in_free_list != 1) { //list only contains dummy
        average_block_size = (bytes_in_free_list) / (items_in_free_list - 1);
        //converting from units to bytes
        min_block_size = min_block_size * sizeof(mem_chunk_t);
        max_block_size = max_block_size * sizeof(mem_chunk_t);
    }
    assert((bytes_in_free_list % (sizeof(mem_chunk_t))) == 0);


    printf("  --- Free list stats ---\n");
    printf("\tCount of items : %d\n", items_in_free_list);
    printf("\tMemory in list : %d (bytes)\n", bytes_in_free_list);
    printf("\t           avg : %g (bytes)\n", average_block_size);
    printf("\t           min : %d (bytes)\n", min_block_size);
    printf("\t           max : %d (bytes)\n", max_block_size);
    printf("\tCalls to sbrk  : %d\n", NumSbrkCalls);
    printf("\tNumber of pages: %d\n", NumPages);
    if (bytes_in_free_list == NumPages * PAGESIZE) {
        printf("  all memory is in the heap -- no leaks are possible\n");
    }
    // if list is empty then just one item in the list.  It is the dummy
    // So min is zero by default.  Do not consider dummy in avg/min/max.
    assert(min_block_size > 0 || items_in_free_list == 1);
}

/* print table of memory in free list 
 *
 * The print should include the dummy item in the list 
 *
 * A unit is the size of one mem_chunk_t structure
 */
void Mem_print(void)
{
    // note position of Rover is not changed by this function
    assert(Rover != NULL && Rover->next != NULL);
    mem_chunk_t *p = Rover;
    mem_chunk_t *start = p;
    do {
        // example format.  Modify for your design
        printf("p=%p, size=%d (units), end=%p, next=%p %s\n", 
                p, p->size_units, p + p->size_units, p->next, 
                p->size_units!=0?"":"<-- dummy");
        p = p->next;
    } while (p != start);
    mem_validate();
}

/* This is an experimental function to attempt to validate the free
 * list when coalescing is used.  It is not clear that these tests
 * will be appropriate for all designs.  If your design utilizes a different
 * approach, that is fine.  You do not need to use this function and you
 * are not required to write your own validate function.
 */
void mem_validate(void)
{
    // note position of Rover is not changed by this function
    assert(Rover != NULL && Rover->next != NULL);
    assert(Rover->size_units >= 0);
    int wrapped = FALSE;
    int found_dummy = FALSE;
    mem_chunk_t *p, *largest, *smallest;

    p = Rover;
    do {
        if (p->size_units == 0) {
            assert(found_dummy == FALSE);
            found_dummy = TRUE;
        } else {
            assert(p->size_units > 0);
        }
        p = p->next;
    } while (p != Rover);
    assert(found_dummy == TRUE);

    if (Coalescing) {
        do {
            if (p >= p->next) {
                // this is not good unless at the one wrap around
                if (wrapped == TRUE) {
                    printf("validate: List is out of order, already found wrap\n");
                    printf("first largest %p, smallest %p\n", largest, smallest);
                    printf("second largest %p, smallest %p\n", p, p->next);
                    assert(wrapped == FALSE);   // stop and use gdb
                } else {
                    wrapped = TRUE;
                    largest = p;
                    smallest = p->next;
                }
            } else {
                assert(p + p->size_units < p->next);
            }
            p = p->next;
        } while (p != Rover);
        assert(wrapped == TRUE);
    }
}
/* vi:set ts=8 sts=4 sw=4 et: */

