#include <stdlib.h>
#include "tlb.h"

// this is gonna be gr8.

/** invalidate all cache lines in the TLB */
void tlb_clear();

/**
 * return 0 if this virtual address does not have a valid
 * mapping in the TLB. Otherwise, return its LRU status: 1
 * if it is the most-recently used, 2 if the next-to-most,
 * etc.
 */
int tlb_peek(size_t va)
{
    /*
        use mlpt translate function to get the physical address
        then go to the set in the cache given by the physical address
        iterate through the set to see if any of the tags match
            if(tags match)
                return LRU status(not sure how to find LRU status)
            else
                return 0
    */
}

/**
 * If this virtual address is in the TLB, return its
 * corresponding physical address. If not, use
 * `translate(va)` to find that address, store the result
 * in the TLB, and return it. In either case, make its
 * cache line the most-recently used in its set.
 *
 * As an exception, if translate(va) returns -1, do not
 * update the TLB: just return -1.
 */
size_t tlb_translate(size_t va);

/** stub for the purpose of testing tlb_* functions */
size_t translate(size_t va) { return va < 0x1234000 ? va + 0x20000 : -1; }

/**
 * Tests given on the assignment description
 */
int main()
{
    tlb_clear();
    assert(tlb_peek(0) == 0);
    assert(tlb_translate(0) == 0x20000);
    assert(tlb_peek(0) == 1);
    assert(tlb_translate(0x200) == 0x20200);
    assert(tlb_peek(0) == 1);
    assert(tlb_peek(0x200) == 1);
    assert(tlb_translate(0x1200) == 0x21200);
    assert(tlb_translate(0x5200) == 0x25200);
    assert(tlb_translate(0x8200) == 0x28200);
    assert(tlb_translate(0x2200) == 0x22200);
    assert(tlb_peek(0x1000) == 1);
    assert(tlb_peek(0x5000) == 1);
    assert(tlb_peek(0x8000) == 1);
    assert(tlb_peek(0x2000) == 1);
    assert(tlb_peek(0x0000) == 1);
    assert(tlb_translate(0x101200) == 0x121200);
    assert(tlb_translate(0x801200) == 0x821200);
    assert(tlb_translate(0x301200) == 0x321200);
    assert(tlb_translate(0x501200) == 0x521200);
    assert(tlb_translate(0xA01200) == 0xA21200);
    assert(tlb_translate(0xA0001200) == -1);
    assert(tlb_peek(0x001200) == 0);
    assert(tlb_peek(0x101200) == 0);
    assert(tlb_peek(0x301200) == 3);
    assert(tlb_peek(0x501200) == 2);
    assert(tlb_peek(0x801200) == 4);
    assert(tlb_peek(0xA01200) == 1);
    assert(tlb_translate(0x301800) == 0x321800);
    assert(tlb_peek(0x001000) == 0);
    assert(tlb_peek(0x101000) == 0);
    assert(tlb_peek(0x301000) == 1);
    assert(tlb_peek(0x501000) == 3);
    assert(tlb_peek(0x801000) == 4);
    assert(tlb_peek(0xA01000) == 2);
}