#include <stdlib.h>
#include <math.h>
#include "tlb.h"
#define WAYS 4
#define SETS 16

/*

    tlb uses VA to index to a cache line struct 
    the cache line struct has a valid bit and lru position and the physical address
    the physical address is got from page tables translate method

    so if you go through the TLB using the VA and you find the thing there, the PA should be there so you can return it
    otherwise you have to look it up with the page table method

*/

// this is gonna be gr8.

// data structure to use in the cache
typedef struct CacheLine
{
    int validBit;
    int lruPos;
    size_t physicalAddr;
    size_t tag;
    size_t setIndex;
    size_t blockOffset;
} CacheLine;

CacheLine *cache[SETS][WAYS];

/** invalidate all cache lines in the TLB */
void tlb_clear()
{
    for (int i = 0; i < SETS; i++)
    {
        for (int j = 0; j < WAYS; j++)
        {
            if (cache[i][j] != NULL)
            {
                cache[i][j]->validBit = 0;
            }
        }
    }
}

/**
 * return 0 if this virtual address does not have a valid
 * mapping in the TLB. Otherwise, return its LRU status: 1
 * if it is the most-recently used, 2 if the next-to-most,
 * etc.
 */
int tlb_peek(size_t va)
{
    int numBOBits = log2(sizeof(CacheLine));
    int numSIBits = log2(sizeof(cache) / (4 * sizeof(CacheLine)));
    int numTagBits = sizeof(size_t) - numBOBits - numSIBits;

    // tag = 64 - SI bits - BO bits
    size_t tag = va >> (numBOBits + numSIBits);

    // chop off the block offset bits and and it with the size of setIndex to get set index bits
    size_t andOnes = ~((0xFFFFFFFFFFFFFFFF >> (numBOBits + numSIBits)) << numSIBits);
    size_t setIndex = (va >> numBOBits) & andOnes;

    for (int i = 0; i < WAYS; i++)
    {
        if (cache[setIndex][i] != NULL && cache[setIndex][i]->tag == tag && cache[setIndex][i]->validBit == 1)
        {
            return cache[setIndex][i]->lruPos;
        }
    }

    return 0;
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
size_t tlb_translate(size_t va)
{

    // check if this virtual address is in the TLB

    int numBOBits = log2(sizeof(CacheLine));
    int numSIBits = log2(sizeof(cache) / (4 * sizeof(CacheLine)));
    int numTagBits = sizeof(size_t) - numBOBits - numSIBits;

    // tag = 64 - SI bits - BO bits
    size_t tag = va >> (numBOBits + numSIBits);

    // chop off the block offset bits and and it with the size of setIndex to get set index bits
    size_t andOnes = ~((0xFFFFFFFFFFFFFFFF >> (numBOBits + numSIBits)) << numSIBits);
    size_t setIndex = (va >> numBOBits) & andOnes;

    size_t blockOffSet = va & ~((0xFFFFFFFFFFFFFFFF >> numBOBits) << numBOBits);

    for (int i = 0; i < WAYS; i++)
    {
        if (cache[setIndex][i] != NULL && cache[setIndex][i]->tag == tag && cache[setIndex][i]->validBit == 1)
        {
            cache[setIndex][i]->lruPos = 1;
            // update all other LRU numbers
            for (int j = 0; j < WAYS; j++)
            {
                if (cache[setIndex][j] != cache[setIndex][i]) // if it is not the thing that we just found outside of this loop, update its LRU
                {
                    if (cache[setIndex][j]->lruPos < cache[setIndex][i]->lruPos) // if it came before the new thing it needs to increment the LRU
                    {
                        cache[setIndex][j]->lruPos += 1;
                    }
                }
            }
            return cache[setIndex][i]->physicalAddr;
        }
    }

    // if it wasnt int the TLB look it up then add it to the TLB
    size_t physicalAddr = translate(va);
    if (physicalAddr != -1)
    {
        CacheLine *newCacheLine = {1, 1, physicalAddr, tag, setIndex, blockOffSet};

        CacheLine *lruCacheLine = NULL;

        // add the new cache line to the cache and evict something the LRU cache line

        // find the LRU cache line
        for (int i = 0; i < WAYS; i++)
        {
            if (lruCacheLine == NULL)
            {
                cache[setIndex][i] = lruCacheLine;
            }
            if (cache[setIndex][i]->lruPos > lruCacheLine->lruPos)
            {
                lruCacheLine = cache[setIndex][i];
            }
        }

        // replace the LRU cache line with the new cache line
        for (int i = 0; i < WAYS; i++)
        {
            if (cache[setIndex][i] == lruCacheLine)
            {
                cache[setIndex][i] = newCacheLine;
                break;
            }
        }

        // update all other LRU numbers
        for (int j = 0; j < WAYS; j++)
        {
            if (cache[setIndex][j] == newCacheLine) // if it is not the thing that we just found outside of this loop, update its LRU
            {
                if (cache[setIndex][j]->lruPos < newCacheLine->lruPos) // if it came before the new thing it needs to increment the LRU
                {
                    cache[setIndex][j]->lruPos += 1;
                }
            }
        }
    }
    else
        return -1;

    return physicalAddr;
}

/** stub for the purpose of testing tlb_* functions */
size_t translate(size_t va) { return va < 0x1234000 ? va + 0x20000 : -1; }

/**
 * Tests given on the assignment description
 */
int main()
{

    // tlb_clear();
    // assert(tlb_peek(0) == 0);
    // assert(tlb_translate(0) == 0x20000);
    // assert(tlb_peek(0) == 1);
    // assert(tlb_translate(0x200) == 0x20200);
    // assert(tlb_peek(0) == 1);
    // assert(tlb_peek(0x200) == 1);
    // assert(tlb_translate(0x1200) == 0x21200);
    // assert(tlb_translate(0x5200) == 0x25200);
    // assert(tlb_translate(0x8200) == 0x28200);
    // assert(tlb_translate(0x2200) == 0x22200);
    // assert(tlb_peek(0x1000) == 1);
    // assert(tlb_peek(0x5000) == 1);
    // assert(tlb_peek(0x8000) == 1);
    // assert(tlb_peek(0x2000) == 1);
    // assert(tlb_peek(0x0000) == 1);
    // assert(tlb_translate(0x101200) == 0x121200);
    // assert(tlb_translate(0x801200) == 0x821200);
    // assert(tlb_translate(0x301200) == 0x321200);
    // assert(tlb_translate(0x501200) == 0x521200);
    // assert(tlb_translate(0xA01200) == 0xA21200);
    // assert(tlb_translate(0xA0001200) == -1);
    // assert(tlb_peek(0x001200) == 0);
    // assert(tlb_peek(0x101200) == 0);
    // assert(tlb_peek(0x301200) == 3);
    // assert(tlb_peek(0x501200) == 2);
    // assert(tlb_peek(0x801200) == 4);
    // assert(tlb_peek(0xA01200) == 1);
    // assert(tlb_translate(0x301800) == 0x321800);
    // assert(tlb_peek(0x001000) == 0);
    // assert(tlb_peek(0x101000) == 0);
    // assert(tlb_peek(0x301000) == 1);
    // assert(tlb_peek(0x501000) == 3);
    // assert(tlb_peek(0x801000) == 4);
    // assert(tlb_peek(0xA01000) == 2);
}
