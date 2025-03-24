/* hashes.c 
 * Lab6: Hash Tables 
 * ECE 2230, Fall 2024
 * 
 *
 * For MP6: you do not need to modify this file.  Use these functions:
 *
 *    hashes_table_pos
 *    hashes_probe_dec
 *         
 *
 *
 *
 * Most notes and functions are from Eternally Confuzzled web page by Julienne
 * Walker (JSW).  Now hosted on:

   https://github.com/ppelleti/jsw-libs/blob/master/Tutorials/jsw_tut_hashing.md

 * ECE 2230 changes:
 *     Changed all functions to assume that hashkey_t is a 32 bit integer.
 *     Beaware that this will fail with a different type of key.
 *     Changed jen_hash to ignore initval
 *
 *     jsw_hash is broken.  Hacked to create static table of random numbers.
 *     tab_hash is based on Tabulation Hash 
 *
 * All but elf_hash work great with random keys.  Perhaps elf would
 * be better with longer strings?
 *
 * With -h double they are all good.  With double hashing primary 
 *    clustering is avoided.  
 *
 * The simpliest hash is also typically poor.  But it allows easy testing with
 * sequential and worst case keys
 *      abs_hash
 *
 * These all work well for random keys, but -i seq is poor due to clustering
 *      djb_hash
 *      djb_hash_xor
 *      sax_hash
 *      fnv_hash
 *
 * These work well for -i seq. Perhaps due to good avalanche properties?
 *      oat_hash
 *      jen_hash
 *      jsw_hash
 *      tab_hash
 *
 * This works poorly for -i seq and -i rand because keys are too short?
 *      elf_hash
 *      weird observation -i rand -m 65537 is poor but 65500 and 65600 are
 *      fine???  Appears that a table size that is a power of two breaks this
 *      hash.  Perhaps because low order bits are not randomized well.
*/

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include "table.h"
#include "hashes.h"

static int HashAlgorithm = ABS_HASH;

unsigned djb_hash(hashkey_t key);
unsigned djb_hash_xor(hashkey_t key);
unsigned sax_hash(hashkey_t key);
unsigned fnv_hash(hashkey_t key);
unsigned oat_hash(hashkey_t key);
unsigned jsw_hash(hashkey_t key);
unsigned elf_hash(hashkey_t key);
unsigned jen_hash(hashkey_t key);
unsigned tab_hash(hashkey_t key);

/* A wrapper function to call the various hashing algorims.  The
 * abs_hash is used by defult unless changed with hashes_configure
 *
 * Pick a hash funcion X_hash() from the hashes.c file.  All hash functions
 * return an unsigned int.  Then mod with table size.  Return signed result.  
 */
int hashes_table_pos(hashkey_t key, int tablesize)
{
    switch (HashAlgorithm) {
        case ABS_HASH: return abs(key) % tablesize;
        case DJB_HASH: return djb_hash(key) % tablesize;
        case SAX_HASH: return sax_hash(key) % tablesize;
        case FNV_HASH: return fnv_hash(key) % tablesize;
        case OAT_HASH: return oat_hash(key) % tablesize;
        case JEN_HASH: return jen_hash(key) % tablesize;
        case JSW_HASH: return jsw_hash(key) % tablesize;
        case ELF_HASH: return elf_hash(key) % tablesize;
        case TAB_HASH: return tab_hash(key) % tablesize;
        default:
                       printf("\n\tInvalid Hash algorithm: %d\n", HashAlgorithm);
                       printf("\t run ./lab6 -f help to see hash function options\n");
    }
    exit(1);
}

/* Function to calculate the probe decrement for double hashing only.  You
 * can assume this function is only called when the probe type is double.
 *
 * Pick a different hash function X_hash() than used in table_hash.  After taking
 * the mod to get a decrement value, make sure the decrement is at least 1.
 *
 * I selected this hash algorithm for no good reason.  Future work is to
 * consider other functions.  But need to make sure that double hashing 
 * uses two different hashing algorithms.
 */
int hashes_probe_dec(hashkey_t key, int size)
{
    int prob_dec = djb_hash_xor(key) % size;
    if (prob_dec < 1) prob_dec = 1;
    return prob_dec;
}

void hashes_configure(int alg)
{
    HashAlgorithm = alg;
    switch (alg) {
        case ABS_HASH: printf("Using hash algorithm abs_hash\n"); break;
        case DJB_HASH: printf("Using hash algorithm djb_hash\n"); break;
        case SAX_HASH: printf("Using hash algorithm sax_hash\n"); break;
        case FNV_HASH: printf("Using hash algorithm fnv_hash\n"); break;
        case OAT_HASH: printf("Using hash algorithm oat_hash\n"); break;
        case JEN_HASH: printf("Using hash algorithm jen_hash\n"); break;
        case JSW_HASH: printf("Using hash algorithm jsw_hash\n"); break;
        case ELF_HASH: printf("Using hash algorithm elf_hash\n"); break;
        case TAB_HASH: printf("Using hash algorithm tab_hash\n"); break;
        default:
                       printf("\n\tInvalid Hash algorithm: %d\n", HashAlgorithm);
                       printf("\t run ./lab6 -f help to see hash function options\n");
                       exit(1);
    }
}

/* #### Bernstein
 
Dan Bernstein created this algorithm and posted it in a newsgroup. It is
known by many as the Chris Torek hash because Chris went a long way
toward popularizing it. Since then it has been used successfully by
many, but despite that the algorithm itself is not very sound when it
comes to avalanche and permutation of the internal state. It has proven
very good for small character keys, where it can outperform algorithms
that result in a more random distribution:

Bernstein's hash should be used with caution. It performs very well in
practice, for no apparently known reasons (much like how the constant 33
does better than more logical constants for no apparent reason), but in
theory it is not up to snuff. Always test this function with sample data
for every application to ensure that it does not encounter a degenerate
case and cause excessive collisions.
*/
unsigned djb_hash(hashkey_t key)
{
    int len = sizeof(key);
    unsigned char *p = (unsigned char *) &key;
    unsigned h = 0;
    int i;

    for (i = 0; i < len; i++)
    {
        h = 33 * h + p[i];
    }
    return h;
}

/* #### Modified Bernstein

A minor update to Bernstein's hash replaces addition with XOR for the
combining step. This change does not appear to be well known or often
used, the original algorithm is still recommended by nearly everyone,
but the new algorithm typically results in a better distribution:

*/
unsigned djb_hash_xor(hashkey_t key)
{
    int len = sizeof(key);
    unsigned char *p = (unsigned char *) &key;
    unsigned h = 0;
    int i;

    for (i = 0; i < len; i++)
    {
        h = 33 * h ^ p[i];
    }

    return h;
}

/* #### Shift-Add-XOR hash

The shift-add-XOR hash was designed as a string hashing function, but
because it is so effective, it works for any data as well with similar
efficiency. The algorithm is surprisingly similar to the rotating hash
except a different choice of constants for the rotation is used, and
addition is a preferred operation for mixing. All in all, this is a
surprisingly powerful and flexible hash. Like many effective hashes, it
will fail tests for avalanche, but that does not seem to affect its
performance in practice.
*/
unsigned sax_hash(hashkey_t key)
{
    int len = sizeof(key);
    unsigned char *p = (unsigned char *) &key;
    unsigned h = 0;
    int i;

    for (i = 0; i < len; i++)
    {
        h ^= (h << 5) + (h >> 2) + p[i];
    }

    return h;
}

/* #### FNV hash

The FNV hash, short for Fowler/Noll/Vo in honor of the creators, is a
very powerful algorithm that, not surprisingly, follows the same lines
as Bernstein's modified hash with carefully chosen constants. This
algorithm has been used in many applications with wonderful results, and
for its simplicity, the FNV hash should be one of the first hashes tried
in an application. It is also recommended that the [FNV website]
(http://www.isthe.com/chongo/tech/comp/fnv/) be visited for
useful descriptions of how to modify the algorithm for various uses.
*/
unsigned fnv_hash(hashkey_t key)
{
    int len = sizeof(key);
    unsigned char *p = (unsigned char *) &key;
    unsigned h = 2166136261;
    int i;

    for (i = 0; i < len; i++)
    {
        h = (h * 16777619) ^ p[i];
    }

    return h;
}

/* #### One-at-a-Time hash

Bob Jenkins is a well known authority on designing hash functions for
table lookup. In fact, one of his hashes is considered state of the art
for lookup, which we will see shortly. A considerably simpler algorithm
of his design is the One-at-a-Time hash:

This algorithm quickly reaches avalanche and performs very well. This
function is another that should be one of the first to be tested in any
application, if not the very first. This algorithm is my personal
preference as a first test hash, and it has seen effective use in
several high level scripting languages as the hash function for their
associative array data type.
*/
unsigned oat_hash(hashkey_t key)
{
    int len = sizeof(key);
    unsigned char *p = (unsigned char *) &key;
    unsigned h = 0;
    int i;

    for (i = 0; i < len; i++)
    {
        h += p[i];
        h += (h << 10);
        h ^= (h >> 6);
    }

    h += (h << 3);
    h ^= (h >> 11);
    h += (h << 15);

    return h;
}

/* #### JSW hash

This is a hash of my own devising that combines a rotating hash with a
table of randomly generated numbers. The algorithm walks through each
byte of the input, and uses it as an index into a table of random
integers generated by a good random number generator. The internal state
is rotated to mix it up a bit, then XORed with the random number from
the table. The result is a uniform distribution if the random numbers
are uniform. The size of the table should match the values in a byte.
For example, if a byte is eight bits then the table would hold 256
random numbers:

In general, this algorithm is among the better ones that I have tested
in terms of both distribution and performance. I may be slightly biased,
but I feel that this function should be on the list of the first to test
in a new application using hash lookup.
*/

// hbr: Make a table of 256 random numbers.  Slightly less than perfect because
// does not consider sign bit.  Would prefer to malloc table, but then need
// to figure out how to free it.
//
// Also, claims by JSW about performance not well supported, unlike tabulation
// hashing.
#define jsw_TABLESIZE 256
static unsigned int jsw_Tab[jsw_TABLESIZE];
static int jsw_InitTable = 0;
void jsw_make_table(void)
{
    for (int i = 0; i < jsw_TABLESIZE; i++) {
        jsw_Tab[i] = (unsigned) (INT_MAX * drand48());
    }
}

unsigned jsw_hash(hashkey_t key)
{
    int len = sizeof(key);
    unsigned char *p = (unsigned char *) &key;
    unsigned h = 16777551;
    int i;

    // hbr hack to automatically make table on first call
    if (jsw_InitTable == 0) {
        jsw_InitTable = 1;
        jsw_make_table();
    }

    for (i = 0; i < len; i++)
    {
        h = (h << 1 | h >> 31) ^ jsw_Tab[p[i]];
    }

    return h;
}


/* #### ELF hash

The ELF hash function has been around for a while, and it is believed to
be one of the better algorithms out there. In my experience, this is
true, though ELF hash does not perform sufficiently better than most of
the other algorithms presented in this tutorial to justify its slightly
more complicated implementation. It should be on your list of first
functions to test in a new lookup implementation:

*/
unsigned elf_hash(hashkey_t key)
{
    int len = sizeof(key);
    unsigned char *p = (unsigned char *) &key;
    unsigned h = 0, g;
    int i;

    for (i = 0; i < len; i++)
    {
        h = (h << 4) + p[i];
        g = h & 0xf0000000L;

        if (g != 0)
        {
            h ^= g >> 24;
        }

        h &= ~g;
    }

    return h;
}

/* #### Jenkins hash

The dreaded Jenkins hash has been thoroughly tested and passes all kinds
of tests for avalanche and permutations. As such it is considered to be
one of the best and most thoroughly analyzed algorithms on the market
presently. Unfortunately, it is also ridiculously complicated compared
to the other hashes examined in this tutorial:

For details on how this algorithm works, feel free to visit Bob Jenkins'
[website](http://burtleburtle.net/bob/).
*/

// if table size is a power of two, then mask result using hashmask instead of 
// using % tablesize
#define hashsize(n) (1U << (n))
#define hashmask(n) (hashsize(n) - 1)

#define mix(a,b,c) \
{ \
    a -= b; a -= c; a ^= (c >> 13); \
    b -= c; b -= a; b ^= (a << 8); \
    c -= a; c -= b; c ^= (b >> 13); \
    a -= b; a -= c; a ^= (c >> 12); \
    b -= c; b -= a; b ^= (a << 16); \
    c -= a; c -= b; c ^= (b >> 5); \
    a -= b; a -= c; a ^= (c >> 3); \
    b -= c; b -= a; b ^= (a << 10); \
    c -= a; c -= b; c ^= (b >> 15); \
}

// hbr original: unsigned jen_hash(unsigned char *k, unsigned length, unsigned initval)
unsigned jen_hash(hashkey_t key)
{
    // hbr
    unsigned length = sizeof(key);
    // Bob's web site claims the idea for initval is to use the previous result 
    // to seed the next calculation.
    //
    // I have just set to zero because this idea does not seem to work for our 
    // application.  We need to be able to repeat the calulation for lookups!
    // So, initval needs to have same value when used to recalculate the hash
    // for a key.
    //
    // initval can be useful if need to change hash values, such as for cockoo
    // hash when a rehash is required.
    unsigned initval = 0;  
    unsigned char *k = (unsigned char *) &key;

    unsigned a, b;
    unsigned c = initval;
    unsigned len = length;

    a = b = 0x9e3779b9;

    while (len >= 12)
    {
        a += (k[0] + ((unsigned)k[1] << 8) + ((unsigned)k[2] << 16) + ((unsigned)k[3] << 24));
        b += (k[4] + ((unsigned)k[5] << 8) + ((unsigned)k[6] << 16) + ((unsigned)k[7] << 24));
        c += (k[8] + ((unsigned)k[9] << 8) + ((unsigned)k[10] << 16) + ((unsigned)k[11] << 24));

        mix(a, b, c);

        k += 12;
        len -= 12;
    }

    c += length;

    switch (len)
    {
    case 11: c += ((unsigned)k[10] << 24);
    case 10: c += ((unsigned)k[9] << 16);
    case 9: c += ((unsigned)k[8] << 8);
        /* First byte of c reserved for length */
    case 8: b += ((unsigned)k[7] << 24);
    case 7: b += ((unsigned)k[6] << 16);
    case 6: b += ((unsigned)k[5] << 8);
    case 5: b += k[4];
    case 4: a += ((unsigned)k[3] << 24);
    case 3: a += ((unsigned)k[2] << 16);
    case 2: a += ((unsigned)k[1] << 8);
    case 1: a += k[0];
    }

    mix(a, b, c);

    return c;
}

/* tabulation hashing.  https://en.wikipedia.org/wiki/Tabulation_hashing
 *
 * This is a universal hashing algorithm.  But not good enough to give 
 * bounded insertion time for cockoo hashing, in general.  
 *
 * Note this is limited to 4 characters (i.e., bytes).  This can be extended to longer
 * strings but the tables also grow large.  Handling long strings needs
 * a different approach.  
 *
 */
#define tab_TABLESIZE 256
#define tab_MAXCHARS  4
static unsigned int tab_Tab[tab_MAXCHARS][tab_TABLESIZE];
static int tab_InitTable = 0;
/* use gcc lib random function.  It generates a long int in range 0 to RAND_MAX.
 *
 * RAND_MAX is the same as INT_MAX for gcc, so the result is 31 bits.
 */
void tab_make_table(void)
{
    for (int i = 0; i < tab_MAXCHARS; i++) {
        for (int j = 0; j < tab_TABLESIZE; j++) {
            tab_Tab[i][j] = random();
        }
    }
}

unsigned int tab_hash(hashkey_t key)
{
    int len = sizeof(key);
    unsigned char *p = (unsigned char *) &key;
    int i;
    unsigned int h = 0;
    assert(len <= tab_MAXCHARS);

    // hack to automatically make table on first call
    if (tab_InitTable == 0) {
        tab_InitTable = 1;
        tab_make_table();
    }

    for (i = 0; i < len; i++)
    {
        h ^= tab_Tab[i][p[i]];
    }
    return h;
}
