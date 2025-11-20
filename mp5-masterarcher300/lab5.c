/* lab5.c 
 * Lab5: Binary Search Trees
 * Drew Buley
 * C20407096
 * ECE 2230, Fall 2024
 *
 * This file contains drivers to test the BST package.
 *
 * The unit driver builds a custom tree and removes selected keys 
 *   -u 1:  run the unit driver with a custom tree
 *
 * There are three drivers to build a tree using bst_insert and then search 
 * for keys in the tree using bst_search.  Use
 *   -o run the driver with an optimal tree
 *   -r run the driver with a randomly generated tree
 *   -p run the driver with a poor order for inserting keys 
 *   -w to set the number of levels in the initial tree
 *   -t to set the number of search trials 
 *
 *   -v to turn on verbose prints.  But make -w and -t small: example -w4 -t10
 *
 * Another test driver tests random inserts and deletes.  This driver builds
 * an initial tree that is random, and then performs insertions and deletions
 * with equal probability.
 *   -e run the equilibrium driver
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

#include "datatypes.h"

/* constants used with Global variables */

#define TRUE 1
#define FALSE 0

/* Global variables for command line parameters */
int Seed = 10312024;
int Verbose = FALSE;
int EquilibriumTest = FALSE;
int OptimalTest = FALSE;
int RandomTest = FALSE;
int PoorTest = FALSE;
int WorstTest = FALSE;
int UnitNumber = -1;
int Levels = 16;
int Trials = 100000;

enum testtypes {OPTIMAL, RANDOM, POOR, WORST};

/* prototypes for functions in this file only */
void getCommandLine(int argc, char **argv);
void equilibriumDriver(void);
void search_driver(int);
void unitDriver(const int *, const int, const int *, const int);
void big_test(void);

void build_optimal(tree_t *T, int levels);
void build_random(tree_t *T, int levels);
void build_poor(tree_t *T, int levels);
int  build_avl_worst(tree_t *T, int levels);
int  build_avl_fib(tree_t *T, int levels);

int main(int argc, char **argv)
{
    getCommandLine(argc, argv);
    printf("Seed: %d\n", Seed);
    srand48(Seed);

    /* ----- build custom tree and remove nodes ----- */
    if (UnitNumber == 0)                        /* enabled with -u flag */
    {
	// example test to remove leaves, 12 and 20, then internal nodes
	// 8, 24, 40 with one child, then 16, 48 with two children
	const int ins[] = {32,16,8,24,4,12,20,28,48,40,56,44,42,46};
	const int del[] = {12,20,8,24,40}; 
        unitDriver(ins, sizeof ins / sizeof(int),
		   del, sizeof del / sizeof(int));
    }
    if (UnitNumber == 1)
    {
	// example tests: (48) is missing its right-left child and
	//                (16) is missing its left-right child
	const int ins1[] = {32,16,48,8,24,40,56,4,20,28,36,44,60};
	const int del1[] = {16,48,32}; 
        unitDriver(ins1, sizeof ins1 / sizeof(int),
		   del1, sizeof del1 / sizeof(int));
    }
    if (UnitNumber == 2)
    {
	// example tests: (16) is missing its right-left child and 
	//                (48) is missing its left-right child
	const int ins1b[] = {32,16,48,8,24,40,56,4,12,28,36,52,60};
	const int del1b[] = {16,48,32}; 
        unitDriver(ins1b, sizeof ins1b / sizeof(int),
		   del1b, sizeof del1b / sizeof(int));
    }
    if (UnitNumber == 3)
    {
	// example deletion with many children
	const int ins[] = {200,100,50,150,25,75,125,175,65,85,135,80,130,140,78,82};
	const int del[] = {100,85,125}; 
        unitDriver(ins, sizeof ins / sizeof(int),
		   del, sizeof del / sizeof(int));
    }
    if (UnitNumber == 4)
    {
        // check replace for duplicate key
        const int ins[] = {10, 10};
	const int del[] = {10};
        unitDriver(ins, sizeof ins/sizeof(int), del, sizeof del/sizeof(int));
    }
    if (UnitNumber == 5)
    {
        // example from homework set
        const int ins[] = {14, 6, 20, 12, 1, 5, 4, 22, 24, 26, 28, 30};
	const int del[] = {10};
        unitDriver(ins, sizeof ins/sizeof(int), del, sizeof del/sizeof(int));
    }
    if (UnitNumber == 6)
    {
        // example with poor shape 
        const int ins[] = {10, 8, 26, 6, 14, 28, 4, 20, 30, 2, 32, 16, 18, 12, 22, 24, 34};
	const int del[] = {10};
        unitDriver(ins, sizeof ins/sizeof(int), del, sizeof del/sizeof(int));
    }
    if (UnitNumber == 7)
    {
        if (Levels > 4) {
            printf("\n\tThis unit driver tests searches and number of comparisons\n");
            printf("\tRun with -u7 -w x for x in the range 0,1,2,3, or 4\n");
            exit(1);
        }
        tree_t *T = bst_construct();
        // build_optimal(T, Levels);
        build_random(T, Levels);
        int n = pow(2, Levels);
        bst_debug_print_tree(T);
        for (int key = 1; key < 2*n; key++) {
            int level = bst_level(T, key);
            mydata_t *dp = bst_search(T, key);
            int count = bst_stats(T);
            if (dp == NULL) {
                printf("key: %d not found on level %d, comparisons: %d", key, level, count);
                if (2*level != count) {
                    printf(" --- ERROR level and count inconsistent!\n");
                } else {
                    printf("\n");
                }
                if (key%2 == 0) {  // all even keys in the tree
                    printf("\n\n\t ERROR key %d not found but is in the tree!\n\n", key);
                }
            } else {
                printf("key: %d found on level %d, comparisons: %d", key, level, count);
                if (2*level + 1 != count) {
                    printf(" --- ERROR level and count inconsistent!\n");
                } else {
                    printf("\n");
                }
                if (key%2 == 1) {  // all odds keys not in the tree
                    printf("\n\n\t ERROR key %d found but is not in the tree!\n\n", key);
                }
            }
        }
        bst_destruct(T);
    }
    if (UnitNumber == 8)
    {
        // this is a mess but should work.  There are about 34 tests.
        big_test();
    }
    if (UnitNumber == 9)
    {
        //create header block and test NULL response of utility functions
        printf("Testing Tree header construction\n");
        tree_t *T = bst_construct();
        assert((T->tree_size == 0) && (T->root == NULL) && (T->recent_comparisons == 0));
        printf("Header successfully created\n");

        mydata_t *testing = bst_search(T, 100);
        assert(testing == NULL);
        printf("Empty search passed\n");

        int level = bst_level(T, 100);
        assert(level == 0);
        printf("Empty level check passed\n");

        int size = bst_size(T);
        assert(size == 0);
        printf("Empty size check passed\n");

        int stats = bst_stats(T);
        assert(stats == 0);
        printf("Empty stats check passed\n");

        int path = bst_internal_path_len(T);
        assert(path == 0);
        printf("Empty path check passed\n");
        printf("All checks passed empty checks\n");
        printf("Freeing tree header, ensure no memory leaks\n");

        bst_destruct(T);
    }
    if (UnitNumber == 10)
    {
        printf("Unit Driver 10: remove all test\n");

        tree_t *T = bst_construct();

        const int ins[] = {10, 8, 26, 6, 14, 28, 4, 20, 30, 2, 32, 16, 18, 12, 22, 24, 34};
        const int del[] = {10, 8, 26, 6, 14, 28, 4, 20, 30, 2, 32, 16, 18, 12, 22, 24, 34};

        int num_inserts = sizeof ins/sizeof(int);
        int num_removes = sizeof del/sizeof(int);

        int *ip = NULL;
        mydata_t *dp;
        int repititions = 2;
        int i, j;
        for ( j = 0; j < repititions; j++) {
            printf("Inserting %d items into tree\n", num_inserts);
            for (i = 0; i < num_inserts; i++) {
                ip = (int *) malloc(sizeof(int));
                *ip = -ins[i]; 
                bst_insert(T, ins[i], ip);
                printf(" -- Test (%d) tree after adding key %d\n", i, ins[i]);
                bst_debug_print_tree(T);
            }
            printf("\n set %d of insertions complete.\n", j + 1);


            printf("Removing %d items from tree\n", num_removes);
            for (i = 0; i < num_removes; i++) {
                printf(" -- Test (%d) about to remove key %d\n", i, del[i]);
                dp = bst_remove(T, del[i]);
                bst_debug_print_tree(T);
                if (dp != NULL)
                    assert(*(int *)dp == -del[i]);
                else
                    printf("\n\nFAILED to find the key in the tree ######\n");
                free(dp);
            }
            printf("\nSet %d of removals done\n", j + 1);
        }
        bst_destruct(T);
    }
    /* ----- large tree tests  ----- */
    if (OptimalTest)                       /* enable with -o flag */
        search_driver(OPTIMAL);
    if (RandomTest)                        /* enable with -r flag */
        search_driver(RANDOM);
    if (PoorTest)                          /* enable with -p flag */
        search_driver(POOR);
    if (WorstTest)                         /* enable with -q flag */
        search_driver(WORST);

    /* test for performance in equilibrium */
    if (EquilibriumTest)                   /* enable with -e flag */
        equilibriumDriver();

    return 0;
}

/* driver to test deletion of keys from custom tree.  
 *
 * ins_keys array with the list of keys to insert into the tree
 *
 * del_keys array with the list of keys to remove from the tree
 */
void unitDriver(const int ins_keys[], const int num_inserts,
        const int del_keys[], const int num_deletes)
{
    int i;
    int *ip;
    tree_t *del_tree;
    mydata_t *dp;

    del_tree = bst_construct();

    printf("\n\n ====== Unit Driver ======\n\n");
    printf("Inserting %d items into tree\n", num_inserts);
    for (i = 0; i < num_inserts; i++) {
        ip = (int *) malloc(sizeof(int));
        *ip = -ins_keys[i]; 
        bst_insert(del_tree, ins_keys[i], ip);
        printf(" -- Test (%d) tree after adding key %d\n", i, ins_keys[i]);
        bst_debug_print_tree(del_tree);
    }
    printf("\nAll insertions complete.\n");
    if (num_deletes > 0)
        printf("Removing %d items from tree\n", num_deletes);
    for (i = 0; i < num_deletes; i++) {
        printf(" -- Test (%d) about to remove key %d\n", i, del_keys[i]);
        dp = bst_remove(del_tree, del_keys[i]);
        bst_debug_print_tree(del_tree);
        if (dp != NULL)
            assert(*(int *)dp == -del_keys[i]);
        else
            printf("\n\nFAILED to find the key in the tree ######\n");
        free(dp);
    }
    /* remove and free all items from tree */
    bst_destruct(del_tree);
}


/* driver to build and test trees.  Creates tree with half of keys
 * in tree and half missing.  Uses search to find random keys.  Note that this
 * algorithm does not delete keys from the tree.
 */
void search_driver(int test_type)
{
    int i;
    int range_num_ints;
    int size;
    int key;
    int ipl, epl;
    int suc_search, suc_trials, unsuc_search, unsuc_trials;
    tree_t *test_tree;
    mydata_t *dp;
    clock_t start, end;

    /* print parameters for this test run */
    printf("\n----- Access driver -----\n");
    printf("  Access trials: %d\n", Trials);
    printf("  Levels for tree: %d\n", Levels);

    range_num_ints = pow(2, Levels);

    /* build tree.  Key range is twice size of tree and each key 
     * is either even or odd.
     */
    test_tree = bst_construct();
    printf("  Build");
    if (test_type == OPTIMAL) {
        printf(" optimal");
        build_optimal(test_tree, Levels);
    } else if (test_type == RANDOM) {
        printf(" random");
        build_random(test_tree, Levels);
    } else if (test_type == POOR) {
        printf(" poor");
        build_poor(test_tree, Levels);
    } else if (test_type == WORST) {
        printf(" worst");
        range_num_ints = build_avl_fib(test_tree, Levels) + 1;
    } else {
        printf("invalid option in search test?\n");
        exit(1);
    }
    printf(" tree with size=%d\n", range_num_ints-1);

    if (Verbose)
        bst_debug_print_tree(test_tree);
    size = bst_size(test_tree);
    assert(size == range_num_ints-1);

    if (Trials > 0) {
        /* search tree to measure successful and unsuccessful search calls */
        suc_search = suc_trials = unsuc_search = unsuc_trials = 0;
        start = clock();
        for (i = 0; i < Trials; i++) {
            /* random key with uniform distribution */
            key = ((int) (drand48() * (range_num_ints * 2 - 1))) + 1;
            dp = bst_search(test_tree, key);
            if (dp == NULL) {
                unsuc_search += bst_stats(test_tree);
                unsuc_trials++;
                if (Verbose)
                    printf("did not find %d after %d comparisons\n", key, bst_stats(test_tree));
            } else {
                suc_search += bst_stats(test_tree);
                suc_trials++;
                assert(*(int *)dp == -key);
                if (Verbose)
                    printf("found %d after %d comparisons \n", key, bst_stats(test_tree));
            }
        }
        end = clock();
        assert(size == bst_size(test_tree));
        ipl = bst_internal_path_len(test_tree);
        epl = ipl + 2 * size;
        printf("  After search exercise, time=%g, tree size=%d\n",
                1000*((double)(end-start))/CLOCKS_PER_SEC, size);
        if (suc_trials > 0)
            printf("    Expect successful search=%g, measured=%g, trials=%d\n", 
                    (2.0*ipl+size)/size,
                    (double) suc_search/suc_trials, suc_trials);
        if (unsuc_trials > 0)
            printf("    Expect unsuccessful search=%g, measured=%g, trials=%d\n", 
                    2.0*epl/(size+1),
                    (double) unsuc_search/unsuc_trials, unsuc_trials);
    }

    bst_debug_validate(test_tree);
    /* remove and free all items from tree */
    bst_destruct(test_tree);
    printf("----- End of search driver -----\n\n");
}
/* driver to test sequence of inserts and deletes.
 */
void equilibriumDriver(void)
{
    int i;
    int range_num_ints;
    int size;
    int key;
    int ipl, epl;
    int suc_search, suc_trials, unsuc_search, unsuc_trials;
    int keys_added, keys_removed;
    int *ip;
    tree_t *test_tree;
    mydata_t *dp;
    clock_t start, end;

    /* print parameters for this test run */
    printf("\n----- Equilibrium test driver -----\n");
    printf("  Trials in equilibrium: %d\n", Trials);
    printf("  Levels in initial tree: %d\n", Levels);

    range_num_ints = pow(2, Levels);

    /* build a random tree as starting point */
    test_tree = bst_construct();
    build_random(test_tree, Levels);
    size = bst_size(test_tree);
    ipl = bst_internal_path_len(test_tree);
    epl = ipl + 2 * size;
    printf("  Initial random tree size=%d\n", size);
    printf("  Expect successful search for initial tree=%g\n", 
            (2.0*ipl+size)/size);
    printf("  Expect unsuccessful search for initial tree=%g\n", 
            2.0*epl/(size+1));
    if (Verbose)
        bst_debug_print_tree(test_tree);

    /* in equilibrium make inserts and removes with equal probability */
    suc_search = suc_trials = unsuc_search = unsuc_trials = 0;
    keys_added = keys_removed = 0;
    start = clock();
    for (i = 0; i < Trials; i++) {
        key = ((int) (drand48() * range_num_ints * 2)) + 1;
        if (Verbose) printf("Trial %d, Key %d", i, key);
        if (drand48() < 0.5) {
            ip = (int *) malloc(sizeof(int));
            *ip = -key;
            /* insert returns 1 if key not found, 0 if found */
            if (bst_insert(test_tree, key, ip) == 1) {
                unsuc_search += bst_stats(test_tree);
                unsuc_trials++;
                keys_added++;
                if (Verbose) printf(" added\n");
            } else {
                suc_search += bst_stats(test_tree);
                suc_trials++;
                if (Verbose) printf(" replaced\n");
            }
        } else {
            dp = bst_remove(test_tree, key);
            if (dp == NULL) {
                if (Verbose) printf(" not found\n");
                unsuc_search += bst_stats(test_tree);
                unsuc_trials++;
            } else {
                if (Verbose) printf(" removed\n");
                suc_search += bst_stats(test_tree);
                suc_trials++;
                keys_removed++;
                assert(*(int *)dp == -key);
                free(dp);
            }
        }
        /* this prints the tree after each trial */
        if (Verbose)
            bst_debug_print_tree(test_tree);
    }
    end = clock();
    /* print the final tree after all trials */
    //if (Verbose)
    //    bst_debug_print_tree(test_tree);

    size += keys_added - keys_removed;
    //printf("added %d removed %d new size %d tree size %d\n", keys_added,
    //        keys_removed, size, bst_size(test_tree));
    assert(size == bst_size(test_tree));
    printf("  After exercise, time=%g, new tree size=%d\n",
            1000*((double)(end-start))/CLOCKS_PER_SEC, size);
    printf("  successful searches during exercise=%g, trials=%d\n", 
            (double) suc_search/suc_trials, suc_trials);
    printf("  unsuccessful searches during exercise=%g, trials=%d\n", 
            (double) unsuc_search/unsuc_trials, unsuc_trials);

    printf("  Validating tree...");
    bst_debug_validate(test_tree);
    printf("passed\n");

    /* test search times for new tree */
    suc_search = suc_trials = unsuc_search = unsuc_trials = 0;
    start = clock();
    for (i = 0; i < Trials; i++) {
        key = ((int) (drand48() * range_num_ints * 2)) + 1;
        dp = bst_search(test_tree, key);
        if (dp == NULL) {
            unsuc_search += bst_stats(test_tree);
            unsuc_trials++;
        } else {
            suc_search += bst_stats(test_tree);
            suc_trials++;
        }
    }
    end = clock();
    size = bst_size(test_tree);
    ipl = bst_internal_path_len(test_tree);
    epl = ipl + 2 * size;
    printf("  After search experiment, time=%g, tree size=%d\n",
            1000*((double)(end-start))/CLOCKS_PER_SEC, size);
    printf("  Expect successful search=%g, measured=%g, trials=%d\n", 
            (2.0*ipl+size)/size,
            (double) suc_search/suc_trials, suc_trials);
    printf("  Expect unsuccessful search=%g, measured=%g, trials=%d\n", 
            2.0*epl/(size+1),
            (double) unsuc_search/unsuc_trials, unsuc_trials);

    /* remove and free all items from tree */
    bst_destruct(test_tree);

    printf("----- End of equilibrium test -----\n\n");
}

/* create one node to be inserted in tree with an even key
 */
void build_one_node(tree_t *T, int key)
{
    int *np = (int *) malloc(sizeof(int));
    *np = -2*key;
    bst_insert(T, 2*key, np);
    //printf("inserted %d\n", 2*key);
}

/* build a complete tree with the lowest level full.  The tree has 
 * 2^(levels)-1 nodes and is perfectly balanced.  The key range is twice the 
 * size of the tree and only the even keys are inserted in the tree.
 */
void build_optimal(tree_t *T, int levels)
{
    int n, start, inc, i, j;
    n = pow(2, levels);
    for (i = 0; i<levels; i++) {
        start = n/pow(2, i + 1); inc = 2 * start;
        for (j = start; j < n; j += inc) {
            build_one_node(T, j);
        }
    }
}
/* build a random tree with 2^(levels)-1 nodes.  The nodes are numbered 2 to 2n and are
 * inserted randomly with a uniform distribution.  Only the even keys are
 * inserted.  Use Knuth shuffle to create random permutation.
 */
void build_random(tree_t *T, int levels)
{
    int nodes = pow(2,levels)-1;
    int *narray;
    int i, key, temp;
    narray = (int *) malloc(nodes*sizeof(int));
    for (i = 0; i<nodes; i++)
        narray[i] = i;
    for (i = 0; i<nodes; i++) {
        key = (int) (drand48() * (nodes - i)) + i;
        assert(i <= key && key < nodes);
        temp = narray[i]; narray[i] = narray[key]; narray[key] = temp;
        build_one_node(T, narray[i]+1);
    }
    free(narray);
}

/* build a tree with 2^(levels)-1 nodes.  The nodes are numbered 2 to 2n and are
 * inserted in a poor order.  Only the even keys are inserted.  
 * 
 * The number of sets is sets
 * The size of a set is set_size
 *
 * Keys in a set are ascending, decending, or zig-zag
 */
void build_poor(tree_t *T, int levels)
{
    int n, start, inc, i, j;
    int low, high;
    n = pow(2, levels);
    for (i = 0; i<levels/2; i++) {
        start = n/pow(2, i + 1); inc = 2 * start;
        for (j = start; j < n; j += inc) {
            build_one_node(T, j);
        }
    }
    int sets = pow(2, levels/2);
    int set_size = pow(2, levels - levels/2);
    for (i = 0; i < sets; i++) {
        low = i*set_size + 1; high = (i+1)*set_size - 1;
        int type = (int) 4*drand48();
        // randomly select type of insertion pattern
        if (type == 0) {  // ascending
            for (j = low; j <= high; j++) {
                build_one_node(T, j);
            }
        } else if (type == 1) {  // decending
            for (j = high; j >= low; j--) {
                build_one_node(T, j);
            }
        } else if (type == 2) {  // zig-zag, low first
            for (j = 0; j < (set_size-1)/2; j++) {
                build_one_node(T, low + j);
                build_one_node(T, high - j);
            }
            if (j*2 != set_size-1) {
                build_one_node(T, low + j);
            }
        } else {  // zig-zag, high first
            for (j = 0; j < (set_size-1)/2; j++) {
                build_one_node(T, high - j);
                build_one_node(T, low + j);
            }
            if (j*2 != set_size-1) {
                build_one_node(T, low + j);
            }
        }
    }
}

/* calculate Fibonacci numbers for use in creating worst
 * case AVL tree
 */
int fibonacci(int n)
{
    assert (n >= 0);
    int n1=1, n2=0;
    int i, fib;
    if (n == 0)      fib = 0;
    else if (n == 1) fib = 1;
    else {
        for (i = 1; i < n; i++) {
            fib = n1 + n2;
            n2 = n1;
            n1 = fib;
        }
    }
    return fib;
}

/* build a worst case tree with addresses in increasing order.
 * This just inserts in increasing order but does not 
 * produce the avl fibonacci tree.
 */
int build_avl_worst(tree_t *T, int levels)
{
    int n, i;
    n = fibonacci(levels + 3) - 1;
    for (i = 1; i <= n; i++) {
        build_one_node(T, i);
    }
    return n;
}

/* the number of nodes in a fibonacci avl tree with given
 * number of levels
 */
int g_h(int levels)
{
    return fibonacci(levels + 3) - 1;
}

/* generates the fibonacci avl tree but in depth first order.
 * if use avl insertions, the structure is lost.
 */
void build_avl_fib_rec(tree_t *T, int levels, int offset)
{
    if (levels == 0)
        build_one_node(T, 1 + offset);
    else if (levels == 1) {
        build_one_node(T, 1 + offset);
        build_one_node(T, 2 + offset);
    } else {
        build_one_node(T, g_h(levels - 2) + 1 + offset);
        build_avl_fib_rec(T, levels - 2, offset);
        build_avl_fib_rec(T, levels - 1, g_h(levels - 2) + 1 + offset);
    }
}

/* auxiliray function to build fibonacci avl tree
 */
int build_avl_fib(tree_t *T, int levels)
{
    int n = g_h(levels);
    build_avl_fib_rec(T, levels, 0);
    return n;
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

    while ((c = getopt(argc, argv, "w:t:s:veoru:pq")) != -1)
        switch(c) {
            case 'w': Levels = atoi(optarg);       break;
            case 't': Trials = atoi(optarg);       break;
            case 's': Seed = atoi(optarg);         break;
            case 'v': Verbose = TRUE;              break;
            case 'e': EquilibriumTest = TRUE;      break;
            case 'o': OptimalTest = TRUE;          break;
            case 'r': RandomTest = TRUE;           break;
            case 'p': PoorTest = TRUE;             break;
            case 'q': WorstTest = TRUE;            break;
            case 'u': UnitNumber = atoi(optarg);   break;
            case '?':
                if (isprint(optopt))
                    fprintf(stderr, "Unknown option %c.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
            default:
                printf("Lab5 command line options\n");
                printf("General options ---------\n");
                printf("  -v        turn on verbose prints (default off)\n");
                printf("  -s 123    seed for random number generator\n");
                printf("  -u 0      run unit test driver with custom tree\n");
                printf("  -o        run search test driver with optimum tree\n");
                printf("  -r        run search test driver with random tree\n");
                printf("  -p        run driver with poor insertion order\n");
                printf("  -e        run equilibrium test driver\n");
                printf("\nOptions for test driver ---------\n");
                printf("  -w 16     levels in tree for drivers\n");
                printf("  -t 50000  number of trials in drivers\n");
                exit(1);
        }
    for (index = optind; index < argc; index++)
        printf("Non-option argument %s\n", argv[index]);
}

/* Here are some additional cases that can be used with the unitDriver 
 *
 * These present a sequence of tests that should cover all
 * the possible test cases.  If all of these tests pass but there is 
 * still a scenario that fails, please let me know.
 *
 * There are 34 tests
 */
void big_test()
{
    printf("case 1.  No children  (three sub cases, parent on left, right, or root)\n");

    const int ins_keys1[] = {4, 2, 6};
    const int del_keys1[] = {2, 6, 4}; 
    unitDriver(ins_keys1, sizeof ins_keys1 / sizeof(int), del_keys1, sizeof del_keys1 / sizeof(int));

    printf("\ncase 2: node 10 has only one child on left\n");

    printf("\tleft of parent\n");
    const int ins_keys2l[] = {20, 10, 5, 3, 7};
    const int del_keys2l[] = {10}; 
    unitDriver(ins_keys2l, sizeof ins_keys2l / sizeof(int), del_keys2l, sizeof del_keys2l / sizeof(int));

    printf("\tright of parent\n");
    const int ins_keys2r[] = {2, 10, 5, 3, 7};
    const int del_keys2r[] = {10}; 
    unitDriver(ins_keys2r, sizeof ins_keys2r / sizeof(int), del_keys2r, sizeof del_keys2r / sizeof(int));

    printf("\troot\n");
    const int ins_keys2[] = {10, 5, 3, 7};
    const int del_keys2[] = {10}; 
    unitDriver(ins_keys2, sizeof ins_keys2 / sizeof(int), del_keys2, sizeof del_keys2 / sizeof(int));

    printf("\nvariation 1:  append 6 to ins_keys\n");
    printf("\tleft of parent\n");
    const int ins_keys2l_1[] = {20, 10, 5, 3, 7, 6};
    const int del_keys2l_1[] = {10}; 
    unitDriver(ins_keys2l_1, sizeof ins_keys2l_1 / sizeof(int), del_keys2l_1, sizeof del_keys2l_1 / sizeof(int));

    printf("\tright of parent\n");
    const int ins_keys2r_1[] = {2, 10, 5, 3, 7, 6};
    const int del_keys2r_1[] = {10}; 
    unitDriver(ins_keys2r_1, sizeof ins_keys2r_1 / sizeof(int), del_keys2r_1, sizeof del_keys2r_1 / sizeof(int));

    printf("\troot\n");
    const int ins_keys2_1[] = {10, 5, 3, 7, 6};
    const int del_keys2_1[] = {10}; 
    unitDriver(ins_keys2_1, sizeof ins_keys2_1 / sizeof(int), del_keys2_1, sizeof del_keys2_1 / sizeof(int));

    //variation 2:  append 9, 8
    printf("\nvariation 2:  append 9, 8 to ins_keys\n");
    printf("\tleft of parent\n");
    const int ins_keys2l_2[] = {20, 10, 5, 3, 7, 6, 9, 8};
    const int del_keys2l_2[] = {10}; 
    unitDriver(ins_keys2l_2, sizeof ins_keys2l_2 / sizeof(int), del_keys2l_2, sizeof del_keys2l_2 / sizeof(int));

    printf("\tright of parent\n");
    const int ins_keys2r_2[] = {2, 10, 5, 3, 7, 6, 9, 8};
    const int del_keys2r_2[] = {10}; 
    unitDriver(ins_keys2r_2, sizeof ins_keys2r_2 / sizeof(int), del_keys2r_2, sizeof del_keys2r_2 / sizeof(int));

    printf("\troot\n");
    const int ins_keys2_2[] = {10, 5, 3, 7, 6, 9, 8};
    const int del_keys2_2[] = {10}; 
    unitDriver(ins_keys2_2, sizeof ins_keys2_2 / sizeof(int), del_keys2_2, sizeof del_keys2_2 / sizeof(int));

    printf("\ncase 3:  node 10 has only one child on right\n");
    printf("\tleft of parent\n");
    const int ins_keys3l[] = {20, 10, 15, 13, 17};
    const int del_keys3l[] = {10}; 
    unitDriver(ins_keys3l, sizeof ins_keys3l / sizeof(int), del_keys3l, sizeof del_keys3l / sizeof(int));

    printf("\tright of parent\n");
    const int ins_keys3r[] = {2, 10, 15, 13, 17};
    const int del_keys3r[] = {10}; 
    unitDriver(ins_keys3r, sizeof ins_keys3r / sizeof(int), del_keys3r, sizeof del_keys3r / sizeof(int));

    printf("\troot\n");
    const int ins_keys3[] = {10, 15, 13, 17};
    const int del_keys3[] = {10}; 
    unitDriver(ins_keys3, sizeof ins_keys3 / sizeof(int), del_keys3, sizeof del_keys3 / sizeof(int));

    printf("\nvariation 1:  append 14\n");
    printf("\tleft of parent\n");
    const int ins_keys3l_1[] = {20, 10, 15, 13, 17, 14};
    const int del_keys3l_1[] = {10}; 
    unitDriver(ins_keys3l_1, sizeof ins_keys3l_1 / sizeof(int), del_keys3l_1, sizeof del_keys3l_1 / sizeof(int));

    printf("\tright of parent\n");
    const int ins_keys3r_1[] = {2, 10, 15, 13, 17, 14};
    const int del_keys3r_1[] = {10}; 
    unitDriver(ins_keys3r_1, sizeof ins_keys3r_1 / sizeof(int), del_keys3r_1, sizeof del_keys3r_1 / sizeof(int));

    printf("\troot\n");
    const int ins_keys3_1[] = {10, 15, 13, 17, 14};
    const int del_keys3_1[] = {10}; 
    unitDriver(ins_keys3_1, sizeof ins_keys3_1 / sizeof(int), del_keys3_1, sizeof del_keys3_1 / sizeof(int));

    //variation 2:  append 11, 12
    printf("\nvariation 2:  append 14, 11, 12\n");
    printf("\tleft of parent\n");
    const int ins_keys3l_2[] = {20, 10, 15, 13, 17, 14, 11, 12};
    const int del_keys3l_2[] = {10}; 
    unitDriver(ins_keys3l_2, sizeof ins_keys3l_2 / sizeof(int), del_keys3l_2, sizeof del_keys3l_2 / sizeof(int));

    printf("\tright of parent\n");
    const int ins_keys3r_2[] = {2, 10, 15, 13, 17, 14, 11, 12};
    const int del_keys3r_2[] = {10}; 
    unitDriver(ins_keys3r_2, sizeof ins_keys3r_2 / sizeof(int), del_keys3r_2, sizeof del_keys3r_2 / sizeof(int));

    printf("\troot\n");
    const int ins_keys3_2[] = {10, 15, 13, 17, 14, 11, 12};
    const int del_keys3_2[] = {10}; 
    unitDriver(ins_keys3_2, sizeof ins_keys3_2 / sizeof(int), del_keys3_2, sizeof del_keys3_2 / sizeof(int));

    printf("\ncase 4:  node 10 has two children but no left-right child\n");
    printf("\tleft of parent\n");
    const int ins_keys4l[] = {20, 10, 5, 3, 15, 13, 17};
    const int del_keys4l[] = {10}; 
    unitDriver(ins_keys4l, sizeof ins_keys4l / sizeof(int), del_keys4l, sizeof del_keys4l / sizeof(int));

    printf("\tright of parent\n");
    const int ins_keys4r[] = {2, 10, 5, 3, 15, 13, 17};
    const int del_keys4r[] = {10}; 
    unitDriver(ins_keys4r, sizeof ins_keys4r / sizeof(int), del_keys4r, sizeof del_keys4r / sizeof(int));

    printf("\troot\n");
    const int ins_keys4[] = {10, 5, 3, 15, 13, 17};
    const int del_keys4[] = {10}; 
    unitDriver(ins_keys4, sizeof ins_keys4 / sizeof(int), del_keys4, sizeof del_keys4 / sizeof(int));

    printf("\tvariation:  append 11, 12, 14\n");
    printf("\tleft of parent\n");
    const int ins_keys4l_1[] = {20, 10, 5, 3, 15, 13, 17, 11, 12, 14};
    const int del_keys4l_1[] = {10}; 
    unitDriver(ins_keys4l_1, sizeof ins_keys4l_1 / sizeof(int), del_keys4l_1, sizeof del_keys4l_1 / sizeof(int));

    printf("\tright of parent\n");
    const int ins_keys4r_1[] = {2, 10, 5, 3, 15, 13, 17, 11, 12, 14};
    const int del_keys4r_1[] = {10}; 
    unitDriver(ins_keys4r_1, sizeof ins_keys4r_1 / sizeof(int), del_keys4r_1, sizeof del_keys4r_1 / sizeof(int));

    printf("\troot\n");
    const int ins_keys4_1[] = {10, 5, 3, 15, 13, 17, 11, 12, 14};
    const int del_keys4_1[] = {10}; 
    unitDriver(ins_keys4_1, sizeof ins_keys4_1 / sizeof(int), del_keys4_1, sizeof del_keys4_1 / sizeof(int));

    printf("\ncase 5:  node 10 has two children but no right-left child\n");
    printf("\tleft of parent\n");
    const int ins_keys5l[] = {20, 10, 5, 3, 15, 7, 17, 6};
    const int del_keys5l[] = {10}; 
    unitDriver(ins_keys5l, sizeof ins_keys5l / sizeof(int), del_keys5l, sizeof del_keys5l / sizeof(int));

    printf("\tright of parent\n");
    const int ins_keys5r[] = {2, 10, 5, 3, 15, 7, 17, 6};
    const int del_keys5r[] = {10}; 
    unitDriver(ins_keys5r, sizeof ins_keys5r / sizeof(int), del_keys5r, sizeof del_keys5r / sizeof(int));

    printf("\troot\n");
    const int ins_keys5[] = {10, 5, 3, 15, 7, 17, 6};
    const int del_keys5[] = {10}; 
    unitDriver(ins_keys5, sizeof ins_keys5 / sizeof(int), del_keys5, sizeof del_keys5 / sizeof(int));

    printf("\tvariation: append 9, 8\n");
    printf("\tleft of parent\n");
    const int ins_keys5l_1[] = {20, 10, 5, 3, 15, 7, 17, 6, 9, 8};
    const int del_keys5l_1[] = {10}; 
    unitDriver(ins_keys5l_1, sizeof ins_keys5l_1 / sizeof(int), del_keys5l_1, sizeof del_keys5l_1 / sizeof(int));

    printf("\tright of parent\n");
    const int ins_keys5r_1[] = {2, 10, 5, 3, 15, 7, 17, 6, 9, 8};
    const int del_keys5r_1[] = {10}; 
    unitDriver(ins_keys5r_1, sizeof ins_keys5r_1 / sizeof(int), del_keys5r_1, sizeof del_keys5r_1 / sizeof(int));

    printf("\troot\n");
    const int ins_keys5_1[] = {10, 5, 3, 15, 7, 17, 6, 9, 8};
    const int del_keys5_1[] = {10}; 
    unitDriver(ins_keys5_1, sizeof ins_keys5_1 / sizeof(int), del_keys5_1, sizeof del_keys5_1 / sizeof(int));
    
    printf("\ncase 6:  node 100 has predecessor and successor that are farther away\n");

    printf("\tleft of parent\n");
    const int ins_keys6l[] = {200, 100, 50, 150, 25, 75, 90, 80, 85, 140, 110, 130, 120};
    const int del_keys6l[] = {100, 90}; 
    unitDriver(ins_keys6l, sizeof ins_keys6l / sizeof(int), del_keys6l, sizeof del_keys6l / sizeof(int));

    printf("\tright of parent\n");
    const int ins_keys6r[] = {2, 100, 50, 150, 25, 75, 90, 80, 85, 140, 110, 130, 120};
    const int del_keys6r[] = {100, 110}; 
    unitDriver(ins_keys6r, sizeof ins_keys6r / sizeof(int), del_keys6r, sizeof del_keys6r / sizeof(int));

    printf("\troot\n");
    const int ins_keys6[] = {100, 50, 150, 25, 75, 90, 80, 85, 140, 110, 130, 120};
    const int del_keys6[] = {100, 90, 110}; 
    unitDriver(ins_keys6, sizeof ins_keys6 / sizeof(int), del_keys6, sizeof del_keys6 / sizeof(int));
}

/* vi:set ts=8 sts=4 sw=4 et: */
