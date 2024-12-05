/* lab6.c 
 * Lab6: Hash Tables 
 * ECE 2230, Fall 2024
 * Drew Buley
 * C20407096
 * 12/5/2024
 *
 * This file contains drivers to test the Hash Table ADT package.
 *
 *   -m to set the table size
 *   -a to set the load factor for -r and -e
 *   -h to set the type of probe sequence {linear|double|quad}
 *
 * The -r driver builds a table using table_insert and then accesses
 * keys in the table using table_retrieve.  Use
 *   -r run the retrieve driver and specifiy the type of initial table keys 
 *      (rand|seq|fold|worst)
 *   -t to set the number of access trials 
 *
 * For debugging tests (also change -m to other values):
 *   -r -t10 -v -m 6 -h linear  
 *   -r -t10 -v -m 7 -h double 
 *   -r -t10 -v -m 8 -h quad 
 *
 * To test troublesome cases with deletions run with 
 *   -d 
 *   -d -h double
 *
 * Tests using the rehash driver.  The table size must be at least 6
 *   -b -v to see many prints
 *   -b -v -m 12 -h linear
 *   -b -v -m 13 -h double 
 *   -b -v -m 16 -h quad 
 *
 *
 * To test the Two Sum Problem use -p X for X in the set {1, 2, 3, 4}
 *    -p 1 -v
 *    -p 2 -v
 *    -p 3 -v
 *    For -p 4 only:
 *        -m to set the size of the array (not the hash table size)
 *        -t to set number of trials, each with a new array
 *
 * To test random inserts and deletes.  This driver builds
 * an initial table with random keys, and then performs insertions and deletions
 * with equal probability.
 *   -e -t 20 -v -m 47 -h {linear|double|quad}
 *
 * For performance analysis test large tables
 *   -r -m {65537|655373} -i {rand|seq} -h {linear|double|} -a {0.9 | 0.7 | etc}
 *   -r -m 65536 -i {rand|seq} -h quad -a {0.9 | 0.7 | etc}
 *   -e -m 65537 -h {linear|double} -t {50000|100000}
 *   -e -m 65536 -h quad -t {50000|100000}
 *
 * Bugs: 
 *    method to create sequential, folded, and worst case table entries only causes poor 
 *       performance with with abs_hash 
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#include "table.h"
#include "hashes.h"

/* constants used with Global variables */

enum TableType_t {RAND, SEQ, FOLD, WORST};

#define LOWID 0
#define MAXID  999999999
#define MINID -999999999
#define PEEK_NOKEY INT_MAX

#define TRUE 1
#define FALSE 0

/* Global variables for command line parameters.  */
int Verbose = FALSE;
static int TableSize = 11;
static int ProbeDec = LINEAR;
static int HashAlg = ABS_HASH;
static double LoadFactor = 0.9;
static int TableType = RAND;
static int RetrieveTest = FALSE;
static int EquilibriumTest = FALSE;
static int RehashTest = FALSE;
static int DeletionTest = FALSE;
static int TwoSumTest = 0;
static int SpecialTest = FALSE;
static int Trials = 50000;
static int Seed = 11172024;

/* prototypes for functions in this file only */
void getCommandLine(int argc, char **argv);
void equilibriumDriver(void);
void RetrieveDriver(void);
void RehashDriver(int);
void TwoSumDriver(int);
void specialDriver(void);
int build_random(table_t *T, int ,int);
int build_seq(table_t *T, int, int);
int build_fold(table_t *T, int, int);
int build_worst(table_t *T, int, int);
void performanceFormulas(double);
int find_first_prime(int number);
void twoSum(const int* nums, const int numsSize, const int target, int *ans1, int *ans2);
void DeletionDriver();
void DeletionFullDriver();

int main(int argc, char **argv)
{
    getCommandLine(argc, argv);
    if (ProbeDec == LINEAR)
        printf("Open addressing with linear probe sequence\n");
    else if (ProbeDec == DOUBLE)
        printf("Open addressing with double hashing\n");
    else if (ProbeDec == QUAD)
        printf("Open addressing with quadratic probe sequence\n");
    hashes_configure(HashAlg);  // defaults to ABS_HASH
    printf("Seed: %d\n", Seed);
    srand48(Seed);

    /* ----- small table tests  ----- */

    if (DeletionTest) {    // enable with -d
        DeletionDriver();
        DeletionFullDriver();
    }
    if (RehashTest)                         /* enable with -b */
        RehashDriver(TableSize);

    /* ----- large table tests  ----- */
    if (RetrieveTest)                        /* enable with -r flag */
        RetrieveDriver();

    /* ----- Two Sum Problem    ----- */
    if (TwoSumTest)                        /* enable with -p flag */
        TwoSumDriver(TwoSumTest);

    /* test for performance in equilibrium */
    if (EquilibriumTest)                   /* enable with -e flag */
        equilibriumDriver();

    /* test special cases */
    if (SpecialTest)                       /*enable with -q flag  */
        specialDriver();

    return 0;
}

void build_table(table_t *test_table, int num_keys)
{
    int probes = -1;
    printf("  Build table with");
    if (TableType == RAND) {
        printf(" %d random keys\n", num_keys);
        probes = build_random(test_table, TableSize, num_keys);
    } else if (TableType == SEQ) {
        printf(" %d sequential keys\n", num_keys);
        probes = build_seq(test_table, TableSize, num_keys);
    } else if (TableType == FOLD) {
        printf(" %d folded keys\n", num_keys);
        probes = build_fold(test_table, TableSize, num_keys);
    } else if (TableType == WORST) {
        printf(" %d worst keys\n", num_keys);
        probes = build_worst(test_table, TableSize, num_keys);
    } else {
        printf("invalid option for table type\n");
        exit(7);
    }
    printf("    The average number of probes for a successful search = %g\n", 
            (double) probes/num_keys);

    if (Verbose)
        table_debug_print(test_table);

    int size = table_entries(test_table);
    assert(size == num_keys);
}


/* This driver tests special edge cases such as deleting a key not in table
 * deleting from empty table, inserting into a full table
 * Inputs: none
 * Outputs: prints success or fail to command line
 */
void specialDriver() {
    int nums[] = {1, 2, 4, 3, 8, 5};
    int bad_test_key = 40;
    int extra_key = 6;
    int check_ins;
    int *ip;

    int nums_size = sizeof(nums)/sizeof(int);
    table_t *table = table_construct(nums_size +1, ProbeDec);
    table_debug_print(table);
    data_t *check_del = table_delete(table, bad_test_key);
    assert(check_del == NULL);

    for (int i = 0; i < nums_size; i++) {
        ip = (int *)malloc(sizeof(int));
        check_ins = table_insert(table, nums[i], ip);
        assert(check_ins == 0);
    }

    printf("table is now full\n");
    table_debug_print(table);
    printf("now trying to insert into full table with an empty cell\n");
    ip = (int *)malloc(sizeof(int));
    check_ins = table_insert(table, bad_test_key, ip);
    table_debug_print(table);
    assert(check_ins == -1);
    free(ip); //as was not inserted into table

    printf("now trying to delete entry not in full table\n");
    check_del = table_delete(table, bad_test_key);
    table_debug_print(table);
    assert(check_del == NULL);

    printf("replacing the empty cell with a deleted cell then attempting an insert followed\n");
    printf("by a delete of a key not in the table\n");
    check_del = table_delete(table, nums[nums_size-1]);
    free(check_del);

    ip = (int *)malloc(sizeof(int));
    table_insert(table, extra_key, ip);

    check_del = table_delete(table, extra_key);
    free(check_del);

    ip = (int *)malloc(sizeof(int));
    table_insert(table, nums[nums_size-1], ip);


    ip = (int*)malloc(sizeof(int));
    check_ins = table_insert(table, bad_test_key, ip);
    assert(check_ins == -1);
    free(ip); //as was not placed into table

    check_del = table_delete(table, bad_test_key);
    assert(check_del == NULL);
    table_debug_print(table);
    table_destruct(table);
}


/* deletion driver with replacement and table size of 7
 *
 * This test is identical to HW Set 8 problem 6.
 *    Generate 5 keys 
 *       k0: location 5
 *       k1: location 5
 *       k2: location 4
 *       k3: location 5
 *       k4: location 5
 *       k5: location 5
 *
 *    Insert keys k0, k1, k2, and k3 into an empty table.  
 *    Remove keys k0, k4, and k1.  
 *    Insert keys k3 (duplicate) and k5.  
 *
 *    Key k3 must be replaced and not occur in the table twice.
 *    Key k5 must be stored in location 5.
 */

void DeletionDriver()
{
#define TABLESIZE 7
    int *ip;
    table_t *H;
    int i, code;
    H = table_construct(TABLESIZE, ProbeDec);
    int key = 0;
    int keys[TABLESIZE];
    int locations[TABLESIZE] = {5, 5, 4, 5, 5, 5, 0};  // fix if change table size

    printf("\n----- Deletion driver for table size 7. Compare to HW8.6-----\n");
    if (ProbeDec == QUAD) {
        printf("The -d driver uses a table size of 7.  Quadratic probing will not work correctly\n");
        exit(1);
    }
    if (ProbeDec != LINEAR) {
        printf("\n\tThis driver works best with linear probing but can work with double\n\n");
    }
    // generate keys for locations.  One less than table size
    for (i = 0; i < TABLESIZE -1; i++) {
        do {
            // printf("key %d trying %s with loc %d\n", i, key, hashes_table_pos(key, TABLESIZE));
            key++;
            if (hashes_table_pos(key, TABLESIZE) == locations[i]) {
                keys[i] = key;
                break;
            }
        } while (1);
        printf("\tk%d is %d for location %d\n", i, keys[i], locations[i]);
    }

    // insert first 4 keys
    int num_ins = 4;
    for (i = 0; i < num_ins; i++) {
        ip = (int *) malloc(sizeof(int));
        *ip = i;
        assert(table_full(H) == 0);
        code = table_insert(H, keys[i], ip);
        ip = NULL;
        assert(code == 0);
    }
    printf("\ntable with k0, k1, k2, and k3\n");
    table_debug_print(H);
    // verify 4 keys in table
    assert(num_ins == table_entries(H));
    for (i = 0; i < num_ins; i++) {
        ip = table_retrieve(H, keys[i]);
        assert(*(int *)ip == i);
        ip = NULL;
    }

    // delete 2 keys in table and one not in table
    const int del[] = {0,4,1}; 
    int num_del = 3;
    for (i = 0; i < num_del; i++) {
        int *rp = table_retrieve(H, keys[del[i]]);
        int *dp = table_delete(H, keys[del[i]]);
        assert(rp == dp);
        free(dp); rp = dp = NULL;
    }
    printf("\ntable after deleting k0 and k1.  Table has k2 and k3\n");
    table_debug_print(H);
    assert(2 == table_entries(H));

    // insert k3.  This should be a replace, not a new insert
    ip = (int *) malloc(sizeof(int));
    *ip = 1919;
    assert(table_full(H) == 0);
    code = table_insert(H, keys[3], ip);
    printf("\ntable after replacing k3.  Verify it is replaced not inserted\n");
    table_debug_print(H);
    ip = NULL;
    assert(code == 1);
    ip = table_retrieve(H, keys[3]);
    assert(*(int *)ip == 1919);
    ip = NULL;
    assert(2 == table_entries(H));

    // insert k5.  This should be inserted in location 5
    ip = (int *) malloc(sizeof(int));
    *ip = 26;
    assert(table_full(H) == 0);
    code = table_insert(H, keys[5], ip);
    printf("\ntable after inserting k5 into position %d\n", locations[5]);
    table_debug_print(H);
    ip = NULL;
    assert(code == 0);
    ip = table_retrieve(H, keys[5]);
    assert(*(int *)ip == 26);
    ip = NULL;
    assert(3 == table_entries(H));
    assert(table_peek(H,locations[5]) == keys[5]);
    table_destruct(H);
}

/* deletion driver with full table.  Linear probing and table size of 7
 * 
 *    Keys, locations
 *       k0, 0
 *       k1, 1
 *       k2, 2
 *       k3, 3
 *       k4, 4
 *       k5, 5
 *       k6, 6
 *       k7, 0
 *       k8, 2
 *    Insert keys k0 through k5 into a new empty table.  
 *    Remove keys k0 through k4.  
 *    Insert keys k6 and k7.  
 *    Retrieve k8 (not found in table).
 */
void DeletionFullDriver()
{
#define TABLESIZE 7
#define DFD_KEYS 9
    int *ip;
    table_t *H;
    int i, code;
    H = table_construct(TABLESIZE, ProbeDec);

    int keys[DFD_KEYS];
    int locations[DFD_KEYS] = {0, 1, 2, 3, 4, 5, 6, 0, 2};  // fix if change num keys 

    printf("\n----- Deletion driver for full table with size 7. Compare to HW8.7 -----\n");
    // generate keys for locations.  One less than table size
    int key = 0;
    for (i = 0; i < DFD_KEYS; i++) {
        do {
            key++;
            if (hashes_table_pos(key, TABLESIZE) == locations[i]) {
                keys[i] = key;
                break;
            }
        } while (1);
        printf("\tk%d is %d for location %d\n", i, keys[i], locations[i]);
    }

    int num_ins = 6;
    int num_del = 5;

    // insert first 6 keys
    for (i = 0; i < num_ins; i++) {
        ip = (int *) malloc(sizeof(int));
        *ip = i;
        assert(table_full(H) == 0);
        code = table_insert(H, keys[i], ip);
        ip = NULL;
        assert(code == 0);
    }
    printf("\ntable that is full\n");
    table_debug_print(H);
    assert(num_ins == table_entries(H));
    assert(table_full(H) == 1);
    for (i = 0; i < num_ins; i++) {
        ip = table_retrieve(H, keys[i]);
        assert(*(int *)ip == i);
        ip = NULL;
    }

    // remove first 5 keys
    for (i = 0; i < num_del; i++) {
        int *rp = table_retrieve(H, keys[i]);
        int *dp = table_delete(H, keys[i]);
        assert(rp == dp);
        free(dp); rp = dp = NULL;
    }
    printf("\ntable after deleting all but one key.  One empty location\n");
    table_debug_print(H);
    assert(1 == table_entries(H));
    assert(table_full(H) == 0);
    assert(table_deletekeys(H) == num_del);

    // insert k6.  This should be inserted in the last empty location 
    ip = (int *) malloc(sizeof(int));
    *ip = 13;
    code = table_insert(H, keys[6], ip);
    printf("\ntable after inserting k6 into last empty location\n");
    table_debug_print(H);
    ip = NULL;
    assert(code == 0);
    assert(2 == table_entries(H));
    ip = table_retrieve(H, keys[6]);
    assert(*(int *)ip == 13);
    ip = NULL;
    assert(table_full(H) == 0);
    assert(table_deletekeys(H) == num_del);

    // insert k7.  This should be inserted in location 0
    ip = (int *) malloc(sizeof(int));
    *ip = 14;
    code = table_insert(H, keys[7], ip);
    int num_probes = table_stats(H);
    printf("\ntable after inserting k7 into position %d.", locations[7]);
    printf(" %d probes were required\n", num_probes);
    if (num_probes != TABLESIZE) {
        printf("\n\tWARNING: should have required %d probes.  Fix is needed\n\n", TABLESIZE);
    }
    table_debug_print(H);
    ip = NULL;
    assert(code == 0);
    assert(3 == table_entries(H));
    ip = table_retrieve(H, keys[7]);
    assert(*(int *)ip == 14);
    ip = NULL;
    assert(table_deletekeys(H) == num_del-1);
    // check unsuccessful search does not have infinite loop
    ip = table_retrieve(H, keys[8]);
    assert(ip == NULL);
    num_probes = table_stats(H);
    printf("Did not find %d after %d probes\n", keys[8], num_probes);
    if (num_probes != TABLESIZE) {
        printf("\n\tWARNING: should have required %d probes.  You need to fix\n\n", TABLESIZE);
    }
    table_destruct(H);
}
/* driver to test small tables.  This is a series of 
 * simple tests and is not exhaustive.
 *
 * input: test_M is the table size for this test run
 */
void RehashDriver(int test_M)
{
    int i, *ip, code;
    table_t *H;

    printf("\n----- Rehash driver -----\n");
    printf("Table size (%d), load factor (%g)\n", TableSize, LoadFactor);
    if (!Verbose) {
        printf("\tre-run with -v for verbose prints\n");
    }
    if (HashAlg != ABS_HASH) {
        printf("\n\t the -b driver only works with -f abs\n");
        exit(1);
    }
    if (test_M < 6) {
        printf("\nRehashDriver designed for table size for at least 6\n");
        printf("Re-run with -m x for x in this range\n");
        exit(1);
    }
    hashkey_t startkey = LOWID + (test_M - LOWID%test_M);
    assert(startkey%test_M == 0);
    assert(test_M > 5);  // tests designed for size at least 6

    H = table_construct(test_M, ProbeDec);
    // fill table sequentially 
    //
    // Fix: need to find keys that hash in sequential locations if
    //      use a hash algorithm other than abs_hash
    //
    for (i = 0; i < test_M-1; i++) {
        ip = (int *) malloc(sizeof(int));
        *ip = 10*i;
        if (Verbose) {
            printf("%d: about to insert %d at position %d", i, startkey+i, 
                    hashes_table_pos(startkey+i, test_M));
            if (ProbeDec == DOUBLE) {
                printf(" with decrement %d", hashes_probe_dec(startkey+i, test_M));
            }
            printf("\n");
        }
        assert(table_full(H) == 0);
        code = table_insert(H, startkey+i, ip);
        ip = NULL;
        assert(code == 0);
        assert(table_entries(H) == i+1);
        assert(table_stats(H) == 1);
        assert(table_peek(H,i) == startkey+i);
    }
    if (Verbose) {
        printf("\nfull table, last entry empty\n");
        table_debug_print(H);
    }
    // tests on empty position
    assert(table_peek(H,i) == PEEK_NOKEY);
    assert(NULL == table_retrieve(H, startkey+i));
    assert(table_stats(H) == 1);
    assert(table_full(H) == 1);
    assert(-1 == table_insert(H, MAXID, NULL));
    // retrieve and replace each entry
    for (i = 0; i < test_M-1; i++) {
        if (Verbose) {
            printf("%d: about to retrieve and then replace %d\n", i, startkey+i);
        }        
        ip = table_retrieve(H, startkey+i);
        assert(*(int *)ip == 10*i);
        ip = NULL;
        assert(table_stats(H) == 1);
        ip = table_retrieve(H, startkey+i+test_M);
        assert(ip == NULL);
        assert(2 <= table_stats(H) && table_stats(H) <= test_M);
        if (ProbeDec == LINEAR)
            assert(table_stats(H) == i+2);
        ip = (int *) malloc(sizeof(int));
        *ip = 99*i;
        assert(1 == table_insert(H, startkey+i, ip));
        ip = NULL;
        ip = table_retrieve(H, startkey+i);
        assert(*(int *)ip == 99*i);
        ip = NULL;
    }
    assert(table_entries(H) == test_M-1);
    assert(table_full(H) == 1);
    // delete tests
    assert(table_deletekeys(H) == 0);
    if (Verbose) {
        printf("about to delete %d\n", startkey+1);
    }
    ip = table_delete(H, startkey+1);
    assert(*(int *)ip == 99);
    free(ip); ip = NULL;
    if (Verbose) {
        printf("\nsecond entry deleted, last entry empty\n");
        table_debug_print(H);
    }
    assert(table_entries(H) == test_M-2);
    assert(table_full(H) == 0);
    assert(table_peek(H,1) == PEEK_NOKEY);
    assert(table_deletekeys(H) == 1);
    ip = table_retrieve(H, startkey+1);  // check key is not there
    assert(ip == NULL);
    assert(table_stats(H) >= 2);
    // attempt to delete keys not in table 
    assert(NULL == table_delete(H, startkey+1));
    assert(NULL == table_delete(H, startkey+test_M-1));
    if (Verbose) {
        printf("\n about to insert %d at %d\n", startkey+1+test_M, 
                hashes_table_pos(startkey+1+test_M, test_M));
    }    // insert key in its place
    ip = (int *) malloc(sizeof(int));
    *ip = 123;
    assert(0 == table_insert(H, startkey+1+test_M, ip));
    ip = NULL;
    assert(table_peek(H,1) == startkey+1+test_M);
    ip = table_retrieve(H, startkey+1+test_M);
    assert(*(int *)ip == 123);
    ip = NULL;
    assert(table_entries(H) == test_M-1);
    assert(table_full(H) == 1);
    assert(table_deletekeys(H) == 0);
    for (i = 2; i < test_M-1; i++) {     // clear out all but two keys
        ip = table_delete(H, startkey+i);
        assert(*(int *)ip == 99*i);
        free(ip); ip = NULL;
    }
    if (Verbose) {
        printf("\n after clearing out all but two keys\n");
        table_debug_print(H);
        printf("\n about to insert %d at %d\n", startkey+test_M-1, hashes_table_pos(startkey+test_M-1, test_M));
    }
    assert(table_entries(H) == 2);
    ip = (int *) malloc(sizeof(int));    // fill last empty
    *ip = 456;
    assert(0 == table_insert(H, startkey+test_M-1, ip));
    ip = NULL;
    if (Verbose) {
        printf("\n after inserting into delete or empty position \n");
        table_debug_print(H);
        printf("\n about to search for %d at %d\n", startkey+test_M, hashes_table_pos(startkey+test_M, test_M));
    }
    assert(table_entries(H) == 3);
    // unsuccessful search when no empty keys
    assert(NULL == table_retrieve(H, startkey+test_M));

    // two keys the collide in position 0
    ip = (int *) malloc(sizeof(int));
    *ip = 77;
    assert(0 == table_insert(H, startkey+test_M, ip));
    ip = (int *) malloc(sizeof(int));
    *ip = 88;
    assert(0 == table_insert(H, startkey+10*test_M, ip));
    ip = NULL;
    assert(table_entries(H) == 5);
    ip = table_delete(H, startkey);  // delete position 0
    assert(*(int *)ip == 0);
    free(ip); ip = NULL;
    assert(table_entries(H) == 4);
    ip = (int *) malloc(sizeof(int));  // replace 
    *ip = 87;
    assert(1 == table_insert(H, startkey+10*test_M, ip));
    ip = NULL;
    assert(table_entries(H) == 4);
    ip = (int *) malloc(sizeof(int));   // put back position 0
    *ip = 76;
    assert(0 == table_insert(H, startkey+20*test_M, ip));
    ip = NULL;
    if (Verbose) {
        printf("\n verify 5 items in table before rehash \n");
        table_debug_print(H);
    }
    assert(table_entries(H) == 5);
    assert(table_peek(H,0) == startkey+20*test_M);
    assert(table_deletekeys(H) == test_M-5);
    // verify 5 items in table
    ip = table_retrieve(H, startkey+1+test_M);
    assert(*(int *)ip == 123);
    ip = table_retrieve(H, startkey+test_M);
    assert(*(int *)ip == 77);
    ip = table_retrieve(H, startkey+10*test_M);
    assert(*(int *)ip == 87);
    ip = table_retrieve(H, startkey+20*test_M);
    assert(*(int *)ip == 76);
    ip = table_retrieve(H, startkey+test_M-1);
    assert(*(int *)ip == 456);
    ip = NULL;
    // rehash
    H = table_rehash(H, test_M);
    assert(table_entries(H) == 5);
    assert(table_deletekeys(H) == 0);
    if (Verbose) {
        printf("\ntable after rehash with 5 items\n");
        table_debug_print(H);
    }
    // verify 5 items in table
    ip = table_retrieve(H, startkey+1+test_M);
    assert(*(int *)ip == 123);
    ip = table_retrieve(H, startkey+test_M);
    assert(*(int *)ip == 77);
    ip = table_retrieve(H, startkey+10*test_M);
    assert(*(int *)ip == 87);
    ip = table_retrieve(H, startkey+20*test_M);
    assert(*(int *)ip == 76);
    ip = table_retrieve(H, startkey+test_M-1);
    assert(*(int *)ip == 456);
    ip = NULL;

    // rehash and increase table size
    // If linear or quad, double the size
    // If double, need new prime
    int new_M = 2*test_M;
    if (ProbeDec == DOUBLE)
        new_M = find_first_prime(new_M);

    H = table_rehash(H, new_M);
    if (Verbose) {
        printf("\nafter increase table to %d with 5 items\n", new_M);
        table_debug_print(H);
    }
    assert(table_entries(H) == 5);
    assert(table_deletekeys(H) == 0);
    // verify 5 keys and information not lost during rehash
    ip = table_retrieve(H, startkey+1+test_M);
    assert(*(int *)ip == 123);
    ip = table_retrieve(H, startkey+test_M);
    assert(*(int *)ip == 77);
    ip = table_retrieve(H, startkey+10*test_M);
    assert(*(int *)ip == 87);
    ip = table_retrieve(H, startkey+20*test_M);
    assert(*(int *)ip == 76);
    ip = table_retrieve(H, startkey+test_M-1);
    assert(*(int *)ip == 456);
    ip = NULL;

    // fill the new larger table
    assert(table_full(H) == 0);
    int new_items = new_M - 1 - 5;
    int base_addr = 2*startkey + 20*test_M*test_M;
    if (base_addr+new_items*test_M > MAXID) {
        printf("re-run -b driver with smaller table size\n");
        exit(1);
    }
    for (i = 0; i < new_items; i++) {
        ip = (int *) malloc(sizeof(int));
        *ip = 10*i;
        code = table_insert(H, base_addr+i*test_M, ip);
        ip = NULL;
        assert(code == 0);
        assert(table_entries(H) == i+1+5);
    }
    assert(table_full(H) == 1);
    assert(table_entries(H) == new_M-1);
    if (Verbose) {
        printf("\nafter larger table filled\n");
        table_debug_print(H);
    }
    // verify new items are found 
    for (i = 0; i < new_items; i++) {
        ip = table_retrieve(H, base_addr+i*test_M);
        assert(*(int *)ip == 10*i);
        ip = NULL;
    }

    // clean up table
    table_destruct(H);
    printf("----- Passed rehash driver -----\n\n");
}

/* driver to build and test tables. Note this driver  
 * does not delete keys from the table.
 */
void RetrieveDriver()
{
    int i;
    int key_range, num_keys;
    int suc_search, suc_trials, unsuc_search, unsuc_trials;
    table_t *test_table;
    hashkey_t key;
    data_t dp;

    /* print parameters for this test run */
    printf("\n----- Retrieve driver -----\n");
    printf("Table size (%d), load factor (%g)\n", TableSize, LoadFactor);
    printf("  Trials: %d\n", Trials);

    num_keys = (int) (TableSize * LoadFactor);
    test_table = table_construct(TableSize, ProbeDec);

    build_table(test_table, num_keys);

    key_range = MAXID - MINID + 1;

    if (Trials > 0) {
        /* access table to measure probes for an unsuccessful search */
        suc_search = suc_trials = unsuc_search = unsuc_trials = 0;
        for (i = 0; i < Trials; i++) {
            /* random key with uniform distribution */
            key = (hashkey_t) (drand48() * key_range) + MINID;
            if (Verbose) {
                printf("%d: looking for %d at position %d", i, key, hashes_table_pos(key, TableSize));
                if (ProbeDec == DOUBLE) {
                    printf(" with decrement %d", hashes_probe_dec(key, TableSize));
                }
                printf("\n");
            }
            dp = table_retrieve(test_table, key);
            if (dp == NULL) {
                unsuc_search += table_stats(test_table);
                unsuc_trials++;
                if (Verbose)
                    printf("\t not found with %d probes\n", 
                            table_stats(test_table));
            } else {
                // this should be very rare
                suc_search += table_stats(test_table);
                suc_trials++;
                if (Verbose)
                    printf("\t\t FOUND with %d probes (this is rare!)\n", 
                            table_stats(test_table));
                assert(*(int *)dp == key);
            }
        }
        assert(num_keys == table_entries(test_table));
        if (suc_trials > 0)
            printf("    Avg probes for successful search = %g measured with %d trials\n", 
                    (double) suc_search/suc_trials, suc_trials);
        if (unsuc_trials > 0)
            printf("    Avg probes for unsuccessful search = %g measured with %d trials\n", 
                    (double) unsuc_search/unsuc_trials, unsuc_trials);
    }

    /* print expected values from analysis with compare to experimental
     * measurements */
    if (TableSize > 100) {
        performanceFormulas(LoadFactor);
    } else {
        printf("\n\tRun with table size at least 101 (-m 101).  Try -m 65537 and 655373\n");
        printf("\tOr, -m 65536 -h quad. Or with table 16 times larger: -m 1048576 -h quad but many duplicates\n");
    }

    /* remove and free all items from table */
    table_destruct(test_table);
    printf("----- End of access driver -----\n\n");
}

/* Given a target, find indices of the two numbers that add up to the target.
 * The index positions must be unique.  The same array entry cannot be used
 * twice.
 *
 * inputs
 *    nums: an array of integers
 *    numsSize: number of items in array
 *    target: there is exactly one solution that sums to target
 * 
 * outputs
 *    ans1 and ans2: index positions such that
 *        nums[ans1] + nums[ans2] == target
 *        ans1 != ans2
 *
 * You cannot change the array called nums.
 *
 * See mp6.pdf for notes on the options for -p 1, 2, 3, or 4.  Or run with -v
 *
 * -t: number of trials for -p 4 only
 * -m: size of array for -p 4 only
 * -v: turn on extra prints
 *
 * Note -m does not set the size of the hash table.
 * Instead you determine an appropriate hash table size.
 *
 * BUGS: Currently does not set hash table sizes correctly for double and quad
 */
void twoSum(const int* nums, const int numsSize, const int target, int *ans1, int *ans2)
{
    int check_ins;
    int complement;
    data_t *check_look; 
    int *ip;

    int table_size = 2*numsSize; //default for linear
    if (ProbeDec == DOUBLE) {
        table_size = find_first_prime(2*numsSize);
    } else if (ProbeDec == QUAD) {
        table_size = 2;
        while (table_size < 2*numsSize) {
            table_size *= 2;
        }
    }

    table_t *table = table_construct(table_size, ProbeDec);

    //place ints as keys in hash table, place num_index as data
    for (int i = 0; i < numsSize; i++) {
        complement = target - nums[i]; //calculate solution 2
        check_look = table_retrieve(table, complement); //check if sol 2 already in hash table
        if (check_look != NULL) { //if it was, check look contains the index it was at
            *ans1 = *(int *)check_look;
            *ans2 = i;
            if (*ans1 == *ans2) {
                continue; //can't be the same index
            }
            table_destruct(table);
            break;
        }
        //solution not already in table
        ip = (int *)malloc(sizeof(int));
        *ip = i;
        check_ins = table_insert(table, nums[i], ip);
        //check return values of this
        assert(check_ins == 0);
    }
}

/* support function to generate arrays for Two Sums Problem
 *
 * You should not change this function
 *
 * inputs
 *    twosumcase: one of 1, 2, 3, or 4
 *    numsSize: size of array to malloc
 *
 * returns:
 *    nums: malloced array of numbers
 *    target: the sum to find
 *    place1 and 2: locations of nums that sum to target
 *
 * Notes
 *     This design creates many sequences, which is a significant problem for
 *     hash tables, especially with linear probing
 */
int *generatetest(int twosumcase, int numsSize, int *target, int *place1, int *place2)
{
    int i, temp;
    int *nums = (int *) malloc(numsSize * sizeof(int));
    if (twosumcase == 4) {
        if (numsSize < 6) {
            printf("   This case designed for list size of 6 or larger\n");
            exit(1);
        }
        int spot1 = 0, spot2 = numsSize-1;
        *target = numsSize * drand48();
        float mix = drand48();
        if (mix < 0.2) *target += 6*numsSize;
        else if (mix < 0.4) *target -= 4*numsSize;
        nums[spot1] = numsSize * drand48();
        nums[spot2] = *target - nums[spot1];
        for (i = 0; i < numsSize - 2; i++) {
            if (i < (numsSize - 2)/2) {
                nums[i+1] = 2.5 * numsSize + i;
            } else {
                nums[i+1] = -numsSize - (i - (numsSize-2)/2);
            }
        }
        if (*target%2==0 && *target/2 != nums[0])
            nums[2] = *target/2;
        // shuffle
        for (i = 0; i<numsSize; i++) {
            int j = (int) (drand48() * (numsSize - i)) + i;
            assert(i <= j && j < numsSize);
            temp = nums[i]; nums[i] = nums[j]; nums[j] = temp;
            if (i == spot1)
                spot1 = j;
            else if (j == spot1)
                spot1 = i;
            if (i == spot2)
                spot2 = j;
            else if (j == spot2)
                spot2 = i;
        }
        if (spot1 > spot2) {
            temp = spot1; spot1 = spot2; spot2 = temp;
        }
        assert(spot1 != spot2);
        assert(0 <= spot1 && spot1 < numsSize && 0 <= spot2 && spot2 < numsSize);
        assert(nums[spot1]+nums[spot2] == *target);
        *place1 = spot1;
        *place2 = spot2;
    } else if (twosumcase == 1) {
        *target = 9;
        nums[0] = 2; nums[1] = 7; nums[2] = 11; nums[3] = 15;
        *place1 = 0; *place2 = 1;
    } else if (twosumcase == 2) {
        *target = 6;
        nums[0] = 3; nums[1] = 2; nums[2] = 4;
        *place1 = 1; *place2 = 2;
    } else if (twosumcase == 3) {
        *target = 6;
        nums[0] = 3; nums[1] = 3;
        *place1 = 0; *place2 = 1;
    } else {
        printf("Invalid case number for Two Sum Driver -p %d\n", twosumcase);
        exit(1);
    }
    return nums;
}
/* Driver to test Two Sum Problem
 *
 * You should not change this function.  You are writing the function twoSum()
 */
void TwoSumDriver(int twosumcase)
{
    int *nums;
    int numsSize;
    int target;
    int ans1 = -1, ans2 = -2;
    int place1, place2;
    int trials = 1;
    int i, j;

    if (twosumcase == 1) {
        numsSize = 4;
    } else if (twosumcase == 2) {
        numsSize = 3;
    } else if (twosumcase == 3) {
        numsSize = 2;
    } else if (twosumcase == 4) {
        trials = Trials;
        numsSize = TableSize;    // redefines input table size as array size instead!
    } else {
        printf("Invalid case number for Two Sum Driver -p %d\n", twosumcase);
        exit(1);
    }
    printf("Two Sum Driver -p %d for -t %d trials array size -m %d\n", TwoSumTest, trials, numsSize);
    assert(0 < trials && 1 < numsSize && numsSize <= pow(2,20));
    int all_pass = TRUE;
    for (i = 0; i < trials; i++) {
        ans1 = -1; ans2 = -2;
        nums = generatetest(twosumcase, numsSize, &target, &place1, &place2);
        if (Verbose) {
            printf("\nTrial %d: expect [%d, %d], target=%d\n", i+1, place1, place2, target);
            if (numsSize <= 20) {
                printf("[ ");
                for (j = 0; j < numsSize - 1; j++) {
                    printf("%d, ", nums[j]);
                }
                printf("%d]\n", nums[j]);
            } else {
                printf("numbers are %d and %d\n", nums[place1], nums[place2]);
            }
        }

        // this is the funtion you are writing
        twoSum(nums, numsSize, target, &ans1, &ans2);

        if ((ans1 == place1 && ans2 == place2) || (ans1 == place2 && ans2 == place1)) {
            if (Verbose) {
                printf("Good job! You found [%d, %d]\n", ans1, ans2);
            }
            free(nums);
        } else {
            printf("Failed Two Sum problem on trial %d of %d\n", i+1, trials);
            printf("    You found [%d, %d]\n", ans1, ans2);
            if (ans1 == ans2)
                printf("    Cannot use same index twice\n");
            if (0 <= ans1 && ans1 < numsSize) {
                printf("      index %d has %d\n", ans1, nums[ans1]);
                if (0 <= ans2 && ans2 < numsSize) {
                    printf("      index %d has %d\n", ans2, nums[ans2]);
                    printf("      your sum is %d\n", nums[ans1]+nums[ans2]);
                }
            }
            printf("    Expected [%d, %d], target=%d\n", place1, place2, target);
            printf("      index %d has %d\n", place1, nums[place1]);
            printf("      index %d has %d\n", place2, nums[place2]);
            all_pass = FALSE;
            free(nums);
            break;
        }
    }
    if (all_pass == TRUE) {
        printf("    All trials passed!\n");
    }

}

/* driver to test sequence of inserts and deletes.
*/
void equilibriumDriver(void)
{
    int i, code;
    int key_range, num_keys;
    int size;
    int ran_index;
    int suc_search, suc_trials, unsuc_search, unsuc_trials;
    int keys_added, keys_removed;
    int *ip;
    table_t *test_table;
    hashkey_t key;
    data_t dp;
    clock_t start, end;

    /* print parameters for this test run */
    printf("\n----- Equilibrium test driver -----\n");
    printf("Table size (%d), load factor (%g)\n", TableSize, LoadFactor);
    printf("  Trials: %d\n", Trials);

    test_table = table_construct(TableSize, ProbeDec);
    num_keys = (int) (TableSize * LoadFactor);

    /* build a table as starting point */
    build_table(test_table, num_keys);
    size = num_keys;

    key_range = MAXID - MINID + 1;
    /* in equilibrium make inserts and removes with equal probability */
    suc_search = suc_trials = unsuc_search = unsuc_trials = 0;
    keys_added = keys_removed = 0;
    start = clock();
    for (i = 0; i < Trials; i++) {
        if (drand48() < 0.5 && table_full(test_table) == FALSE) {
            // insert only if table not full
            key = (hashkey_t) (drand48() * key_range) + MINID;
            ip = (int *) malloc(sizeof(int));
            *ip = key;
            /* insert returns 0 if key not found, 1 if older key found */
            if (Verbose) printf("Trial %d, Insert Key %d", i, key);
            code = table_insert(test_table, key, ip);
            if (code == 0) {
                /* key was not in table so added */
                unsuc_search += table_stats(test_table);
                unsuc_trials++;
                keys_added++;
                if (Verbose) printf(" added\n");
            } else if (code == 1) {
                suc_search += table_stats(test_table);
                suc_trials++;
                if (Verbose) printf(" replaced (rare!)\n");
            } else {
                printf("!!!Trial %d failed to insert key (%d) with code (%d)\n", i, key, code);
                exit(10);
            }
        } else if (table_entries(test_table) > TableSize/4) {
            // delete only if table is at least 25% full
            // why 25%?  Would 10% be better?  Lower than 10% will
            // be computationally expensive
            do {
                ran_index = (int) (drand48() * TableSize);
                key = table_peek(test_table, ran_index);
            } while (key == PEEK_NOKEY);
            if (Verbose) printf("Trial %d, Delete Key %d", i, key);
            if (key < MINID || MAXID < key)
            {
                printf("\n\n  table peek failed: invalid key (%d) during trial (%d)\n", key, i);
                exit(12);
            }
            dp = table_delete(test_table, key);
            if (dp != NULL) {
                if (Verbose) printf(" removed\n");
                suc_search += table_stats(test_table);
                suc_trials++;
                keys_removed++;
                assert(*(int *)dp == key);
                free(dp);
            } else {
                printf("!!! failed to find key (%d) in table, trial (%d)!\n", key, i);
                printf("this is a catastrophic error!!!\n");
                exit(11);
            }
        }
    }
    end = clock();

    if (Verbose) {
        printf("Table after equilibrium trials\n");
        table_debug_print(test_table);
    }

    size += keys_added - keys_removed;
    printf("  Keys added (%d), removed (%d) new size should be (%d) and is (%d)\n",
            keys_added, keys_removed, size, table_entries(test_table));
    assert(size == table_entries(test_table));
    printf("  After exercise, time=%g \n",
            1000*((double)(end-start))/CLOCKS_PER_SEC);
    printf("  successful searches during exercise=%g, trials=%d\n", 
            (double) suc_search/suc_trials, suc_trials);
    printf("  unsuccessful searches during exercise=%g, trials=%d\n", 
            (double) unsuc_search/unsuc_trials, unsuc_trials);


    /* test access times for new table */

    suc_search = suc_trials = unsuc_search = unsuc_trials = 0;
    start = clock();
    /* check each position in table for key */
    for (i = 0; i < TableSize; i++) {
        key = table_peek(test_table, i);
        if (key != PEEK_NOKEY) {
            assert(MINID <= key && key <= MAXID);
            dp = table_retrieve(test_table, key);
            if (dp == NULL) {
                printf("Failed to find key (%d) but it is in location (%d)\n", 
                        key, i);
                exit(16);
            } else {
                suc_search += table_stats(test_table);
                suc_trials++;
                assert(*(int *)dp == key);
            }
        }
    }
    for (i = 0; i < Trials; i++) {
        /* random key with uniform distribution */
        key = (hashkey_t) (drand48() * key_range) + MINID;
        dp = table_retrieve(test_table, key);
        if (dp == NULL) {
            unsuc_search += table_stats(test_table);
            unsuc_trials++;
        } else {
            // this should be very rare
            assert(*(int *)dp == key);
        }
    }
    end = clock();
    size = table_entries(test_table);
    printf("  After retrieve experiment, time=%g\n",
            1000*((double)(end-start))/CLOCKS_PER_SEC);
    printf("  New load factor = %g\n", (double) size/TableSize);
    printf("  Percent empty locations marked deleted = %g\n",
            (double) 100.0 * table_deletekeys(test_table)
            / (TableSize - table_entries(test_table)));

    printf("   Measured avg probes for successful search=%g, trials=%d\n", 
            (double) suc_search/suc_trials, suc_trials);

    printf("   Measured avg probes for unsuccessful search=%g, trials=%d\n", 
            (double) unsuc_search/unsuc_trials, unsuc_trials);
    if (TableSize > 100) {
        printf("    Do deletions increase avg number of probes?\n");
        performanceFormulas((double) size/TableSize);
    } else {
        printf("\n\tRun with table size at least 101 (-m 101).  Try -m 65537 and 655373\n\n");
    }

    /* rehash and retest table */
    printf("  Rehash table\n");
    test_table = table_rehash(test_table, TableSize);
    /* number entries in table should not change */
    assert(size == table_entries(test_table));
    /* rehashing must clear all entries marked for deletion */
    assert(0 == table_deletekeys(test_table));

    /* test access times for rehashed table */

    suc_search = suc_trials = unsuc_search = unsuc_trials = 0;
    start = clock();
    /* check each position in table for key */
    for (i = 0; i < TableSize; i++) {
        key = table_peek(test_table, i);
        if (key != PEEK_NOKEY) {
            assert(MINID <= key && key <= MAXID);
            dp = table_retrieve(test_table, key);
            if (dp == NULL) {
                printf("Failed to find key (%d) after rehash but it is in location (%d)\n", 
                        key, i);
                exit(26);
            } else {
                suc_search += table_stats(test_table);
                suc_trials++;
                assert(*(int *)dp == key);
            }
        }
    }
    for (i = 0; i < Trials; i++) {
        /* random key with uniform distribution */
        key = (hashkey_t) (drand48() * key_range) + MINID;
        dp = table_retrieve(test_table, key);
        if (dp == NULL) {
            unsuc_search += table_stats(test_table);
            unsuc_trials++;
        } else {
            // this should be very rare
            assert(*(int *)dp == key);
        }
    }
    end = clock();
    size = table_entries(test_table);
    printf("  After rehash, time=%g\n",
            1000*((double)(end-start))/CLOCKS_PER_SEC);
    printf("   Measured avg probes for successful search=%g, trials=%d\n", 
            (double) suc_search/suc_trials, suc_trials);

    printf("   Measured avg probes for unsuccessful search=%g, trials=%d\n", 
            (double) unsuc_search/unsuc_trials, unsuc_trials);

    /* remove and free all items from table */
    table_destruct(test_table);

    printf("----- End of equilibrium test -----\n\n");
}

/* build a table with random keys.  The keys are generated with a uniform
 * distribution.  
 */
int build_random(table_t *T, int table_size, int num_addr)
{
    hashkey_t key;
    int i, range, code;
    int probes = 0;
    int *ip;
    range = MAXID - MINID + 1;
    for (i = 0; i < num_addr; i++) {
        key = (hashkey_t) (drand48() * range) + MINID;
        assert(MINID <= key && key <= MAXID);
        ip = (int *) malloc(sizeof(int));
        *ip = key;
        code = table_insert(T, key, ip);
        if (code == 1) {
            i--;   // since does not increase size of table
            // replaced.  The chances should be very small
            if (num_addr < range/10000) {
                printf("during random build generated duplicate key (%d) on trial (%d)\n", key, i);
                printf("this should be unlikely: if see more than a few you have a problem\n");
            }
        }
        else if (code != 0) {
            printf("build of random table failed code (%d) index (%d) key (%d)\n",
                    code, i, key);
            exit(2);
        }
        probes += table_stats(T);
    }
    return probes;
}

/* build a table with sequential keys.  The starting address is random.  The
 * keys are are in adjacent table locations.
 */
int build_seq(table_t *T, int table_size, int num_addr)
{
    hashkey_t key;
    int i, range, starting, code;
    int *ip;
    int probes = 0;
    range = MAXID - MINID + 1;
    starting = (int) (drand48() * range) + MINID;
    if (starting >= MAXID - table_size)
        starting -= table_size;
    for (i = starting; i < starting + num_addr; i++) {
        assert(MINID <= i && i <= MAXID);
        key = i;
        ip = (int *) malloc(sizeof(int));
        *ip = i;
        code = table_insert(T, key, ip);
        if (code != 0) {
            printf("build of sequential table failed code (%d) index (%d) key (%d)\n",
                    code, i - starting, key);
            exit(3);
        }
        probes += table_stats(T);
    }
    return probes;
}

/* build a table with folded keys.  The starting address is random.  The first
 * set of keys are sequential, and the second set hashes to the same table
 * locations as the first set.
 */
int build_fold(table_t *T, int table_size, int num_addr)
{
    int i, range, starting, code;
    int probes = 0;
    int *ip;
    range = MAXID - MINID + 1;
    starting = (int) (drand48() * range) + MINID;
    if (starting <= MINID + table_size)
        starting += table_size;
    if (starting >= MAXID - table_size)
        starting -= table_size;
    for (i = starting; i > starting - num_addr/2; i--) {
        assert(MINID <= i && i <= MAXID);
        ip = (int *) malloc(sizeof(int));
        *ip = i;
        code = table_insert(T, i, ip);
        if (code != 0) {
            printf("build of first phase of folded table failed code (%d) index (%d) key (%d)\n",
                    code, i - starting, i);
            exit(4);
        }
        probes += table_stats(T);
    }
    for (i = starting + table_size; i > starting + table_size - (num_addr+1)/2; i--) {
        assert(MINID <= i && i <= MAXID);
        ip = (int *) malloc(sizeof(int));
        *ip = i;
        code = table_insert(T, i, ip);
        if (code != 0) {
            printf("build of second phase of folded table failed code (%d) index (%d) key (%d)\n",
                    code, i - starting, i);
            exit(5);
        }
        probes += table_stats(T);
    }
    return probes;
}

/* build a table with worst keys.  Insert keys that hash to the same table
 * location.  Protects against invalid keys by wrapping around if the total
 * number of addresses times the table size is large.
 */
int build_worst(table_t *T, int table_size, int num_addr)
{
    hashkey_t key = MAXID;
    int i, batches = 0, code;
    int probes = 0;
    int *ip;
    for (i = 0; i < num_addr; i++) {
        assert(MINID <= key && key <= MAXID);
        ip = (int *) malloc(sizeof(int));
        *ip = key;
        code = table_insert(T, key, ip);
        if (code != 0) {
            printf("build of worst table failed: code (%d) index (%d) key (%d) batch (%d)\n",
                    code, i, key, batches);
            exit(6);
        }
        if (key < MINID + table_size) {
            batches++;
            printf("batch %d\n", batches);
            key = MAXID - batches;
        }
        else
            key -= table_size;
        probes += table_stats(T);
    }
    return probes;
}

/* return first prime number at number or greater
 *
 * There is at least one prime p such that n < p < 2n
 * for n>=25, n < p < 1.2n
 */
int find_first_prime(int number)
{
    int i, foundfactor;
    double upper;
    assert(number > 1);
    // if even move to next odd
    if (number % 2 == 0)
        number++;
    do {
        foundfactor = 0;      // assume number is prime
        upper = sqrt(number);
        for (i = 3; i < upper + 1; i += 2)
            if (number % i == 0) {
                foundfactor = 1;
                number += 2;  // only test odds
                break;
            }
    } while (foundfactor);
    return number;
}


/* print performance evaulation formulas from Standish pg. 479 and pg 484
 *
 * Added additional formulas for linear probing and sequential, folded, and
 * worst addressing.  Also, for quadratic with worst addressing since it
 * behaves the same as linear in this case.  Formulas for the other cases
 * are unknown.
 */
void performanceFormulas(double load_factor)
{
    int n = TableSize * load_factor;
    if (TableType == RAND) {
        if (ProbeDec == LINEAR) {
            printf("--- Linear probe sequence performance formulas ---\n");
            printf("    Expected probes for successful search %g\n",
                    0.5 * (1.0 + 1.0/(1.0 - load_factor)));
            printf("    Expected probes for unsuccessful search %g\n",
                    0.5 * (1.0 + pow(1.0/(1.0 - load_factor),2)));
        }
        else if (ProbeDec == DOUBLE) {
            printf("--- Double hashing performance formulas ---\n");
            printf("    Expected probes for successful search %g\n",
                    (1.0/load_factor) * log(1.0/(1.0 - load_factor)));
            printf("    Expected probes for unsuccessful search %g\n",
                    1.0/(1.0 - load_factor));
        }
        else if (ProbeDec == QUAD) {
            printf("--- Quadratic probe sequence performance formulas ---\n");
            printf("    Expected probes for successful search %g\n",
                    1.0 - log(1.0 - load_factor) - load_factor/2.0);
            printf("    Expected probes for unsuccessful search %g\n",
                    1.0/(1.0 - load_factor) - load_factor - log(1.0 - load_factor));
        }
    }
    else if (TableType == SEQ) {
        if (HashAlg == ABS_HASH && ProbeDec == LINEAR) {
            printf("--- Linear probe sequence performance formulas ---\n");
            printf("    Expected probes for successful search 1\n");
            printf("    Expected probes for unsuccessful search %g\n",
                    n * load_factor / 2.0 + load_factor/2.0 + 1);
        }
    }
    else if (TableType == FOLD) {
        if (HashAlg == ABS_HASH && ProbeDec == LINEAR) {
            printf("--- Linear probe sequence performance formulas ---\n");
            printf("    Expected probes for successful search %g\n",
                    n / 4.0 + 1);
            printf("    Expected probes for unsuccessful search %g\n",
                    n * load_factor / 2.0 + load_factor/2.0 + 1);
        }
    }
    else if (TableType == WORST) {
        if (HashAlg == ABS_HASH && ProbeDec == LINEAR) {
            printf("--- Linear probe sequence performance formulas ---\n");
            printf("    Expected probes for successful search %g\n",
                    n / 2.0 + 0.5);
            printf("    Expected probes for unsuccessful search %g\n",
                    n * load_factor / 2.0 + load_factor/2.0 + 1);
        }
        else if (HashAlg == ABS_HASH && ProbeDec == QUAD) {
            printf("--- Quadratic probe sequence performance formulas ---\n");
            printf("    Expected probes for successful search %g\n",
                    n / 2.0 + 0.5);
        }
    }
}

/* read in command line arguments and store in global variables for easy
 * access by other functions.
 */
void getCommandLine(int argc, char **argv)
{
    /* optopt--if an unknown option character is found
     * optind--index of next element in argv 
     * optarg--argument for option that requires argument 
     * "x:" colon after x means argument required
     */
    int c;
    int index;

    while ((c = getopt(argc, argv, "m:a:h:f:i:t:s:p:qerbdv")) != -1)
        switch(c) {
            case 'm': TableSize = atoi(optarg);      break;
            case 'a': LoadFactor = atof(optarg);     break;
            case 's': Seed = atoi(optarg);           break;
            case 't': Trials = atoi(optarg);         break;
            case 'v': Verbose = TRUE;                break;
            case 'e': EquilibriumTest = TRUE;        break;
            case 'r': RetrieveTest = TRUE;           break;
            case 'b': RehashTest = TRUE;             break;
            case 'd': DeletionTest = TRUE;           break;
            case 'p': TwoSumTest = atoi(optarg);     break;
            case 'q': SpecialTest = TRUE;            break;
            case 'h':
                      if (strcmp(optarg, "linear") == 0)
                          ProbeDec = LINEAR;
                      else if (strcmp(optarg, "double") == 0)
                          ProbeDec = DOUBLE;
                      else if (strcmp(optarg, "quad") == 0)
                          ProbeDec = QUAD;
                      else {
                          fprintf(stderr, "invalid type of probing decrement: %s\n", optarg);
                          fprintf(stderr, "must be {linear | double | quad}\n");
                          exit(1);
                      }
                      break;
            case 'f':
                      if (strcmp(optarg, "abs") == 0)
                          HashAlg = ABS_HASH;
                      else if (strcmp(optarg, "djb") == 0)
                          HashAlg = DJB_HASH;
                      else if (strcmp(optarg, "sax") == 0)
                          HashAlg = SAX_HASH;
                      else if (strcmp(optarg, "fnv") == 0)
                          HashAlg = FNV_HASH;
                      else if (strcmp(optarg, "oat") == 0)
                          HashAlg = OAT_HASH;
                      else if (strcmp(optarg, "jen") == 0)
                          HashAlg = JEN_HASH;
                      else if (strcmp(optarg, "jsw") == 0)
                          HashAlg = JSW_HASH;
                      else if (strcmp(optarg, "elf") == 0)
                          HashAlg = ELF_HASH;
                      else if (strcmp(optarg, "tab") == 0)
                          HashAlg = TAB_HASH;
                      else {
                          fprintf(stderr, "invalid Hash Algorithm : %s\n", optarg);
                          fprintf(stderr, "must be: -f abs|djb|sax|fnv|oat|jen|jsw|elf|tab\n");
                          exit(1);
                      }
                      break;
            case 'i':
                      if (strcmp(optarg, "rand") == 0)
                          TableType = RAND;
                      else if (strcmp(optarg, "seq") == 0)
                          TableType = SEQ;
                      else if (strcmp(optarg, "fold") == 0)
                          TableType = FOLD;
                      else if (strcmp(optarg, "worst") == 0)
                          TableType = WORST;
                      else {
                          fprintf(stderr, "invalid type of address generation: %s\n", optarg);
                          fprintf(stderr, "must be {rand | seq | fold | worst}\n");
                          exit(1);
                      }
                      break;
            case '?':
                      if (isprint(optopt))
                          fprintf(stderr, "Unknown option %c.\n", optopt);
                      else
                          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
            default:
                      printf("Lab6 command line options\n");
                      printf("General options ---------\n");
                      printf("  -m 11     table size\n");
                      printf("  -a 0.9    load factor\n");
                      printf("  -h linear|double|quad\n");
                      printf("            Type of probing decrement\n");
                      printf("  -r        run retrieve test driver \n");
                      printf("  -d        run deletion test drivers\n");
                      printf("  -b        run basic test driver \n");
                      printf("  -p x      run Two Sums Problem driver for x=1,2,3, or 4\n");
                      printf("                 use -m array size -t test trials\n");
                      printf("  -e        run equilibrium test driver\n");
                      printf("  -i rand|seq|fold|worst\n");
                      printf("            type of keys for retrieve test driver \n");
                      printf("  -f abs|djb|sax|fnv|oat|jen|jsw|elf|tab\n");
                      printf("\nOptions for test driver ---------\n");
                      printf("  -t 50000  number of trials in drivers\n");
                      printf("  -v        turn on verbose prints (default off)\n");
                      printf("  -s 26214  seed for random number generator\n");
                      exit(1);
        }
    for (index = optind; index < argc; index++)
        printf("Non-option argument %s\n", argv[index]);
}

/* vi:set ts=8 sts=4 sw=4 et: */
