/* geninput.c
 * MP3
 * Harlan Russell
 * ECE 2230, Fall 2024
 *
 * Purpose: Generate an input file for MP3 
 * Assumptions: Input file is for testing sort.  Creates
 *              an unsorted list, uses REQ to insert items, and
 *              calls SORT to sort the list
 *
 * Command line arguments:
 *    1st -- number of records to create
 *    2nd -- type of list to create
 *           1: random addresses [0, 3/4 number records)
 *           2: assending and sequential
 *           3: descending and sequential
 *    3rd -- type of sort
 *           1: Insertion
 *           2: Recursive Selection
 *           3: Iterative Selection
 *           4: Merge
 *           5: qsort 
 *
 * Pipe the output of this program into lab3. For example
 *     ./geninput 10000 1 1 | ./lab3
 *
 * See also mp3test.sh 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define TRUE  1
#define FALSE 0

int main(int argc, char *argv[])
{
    int records = 0, list_type = -1, sort_type = -1, include_duplicates = TRUE;
    int addr_range;
    int eth_addr;
    int seed = 9192024;

    if (argc != 4 && argc != 5) {
        printf("Usage: ./geninput listsize listype sorttype [seed]\n");
        printf("\tlistsize: number of records to create\n");
        printf("\tlisttype: {1, 2, 3} for random, ascending or descending\n");
        printf("\tsorttype: 1: Insertion\n");
        printf("\t          2: Recursive Selection\n");
        printf("\t          3: Iterative Selection\n");
        printf("\t          4: Merge\n");
        printf("\t          5: qsort\n");
        printf("\tseed: optional seed for random number generator\n");
        exit(1);
    }
    records = atoi(argv[1]);
    if (records < 2) {
        printf("genniput has invalid number records: %d\n", records);
        exit(2);
    }
    list_type = atoi(argv[2]);
    sort_type = atoi(argv[3]);
    if (sort_type < 1 || sort_type > 5) {
        printf("genniput has invalid type of sort: %d\n", sort_type);
        exit(2);
    }
    if (argc == 5) {
        int temp_seed = -1;
        temp_seed = atoi(argv[4]);
        if (temp_seed > 0)
            seed = temp_seed;
    }
    // first line needs to be size of task list
    printf("3  size of task list\n");
    if (records <= 20)
        printf("Seed %d\n", seed);
    addr_range = records * 0.5;
    srand48(seed);

    // change include_duplicates to FALSE if want to test
    // sorting random numbers with no ties.
    int i;
    if (list_type == 1) {
        if (include_duplicates == TRUE) {
            // random addresses, some duplicates 
            // make sure front and back have duplicates for larger list sizes
            for (i = 0; i < records; i++) {
                if ((records >= 8) && (i == records/4 || i == records/2 || i == 3*records/4)) {
                    eth_addr = records;
                } else if ((records >= 9) && (i == records/3 || i == 2*records/3)) {
                    eth_addr = 0;
                } else {
                    // 1 to addr_range
                    eth_addr = (int) (addr_range * drand48()) + 1;
                }
                printf("REQ %d\n", eth_addr);
            }
        } else {
            // random permutation, no duplicates 
            int * narray = (int *) malloc(records*sizeof(int));
            for (i = 0; i<records; i++)
                narray[i] = i;
            for (i = 0; i<records; i++) {
                int key = (int) (drand48() * (records - i)) + i;
                assert(i <= key && key < records);
                int temp = narray[i]; narray[i] = narray[key]; narray[key] = temp;
                printf("REQ %d\n", narray[i]+1);
            }
            free(narray);
        }
    }
    else if (list_type == 2) {
        // ascending addresses
        for (i = 0; i < records; i++) {
            eth_addr = i+1;
            printf("REQ %d\n", eth_addr);
        }
    }
    else if (list_type == 3) {
        // descending addresses expect for qsort
        if (sort_type != 5) {
            for (i = 0; i < records; i++) {
                eth_addr = records - i;
                printf("REQ %d\n", eth_addr);
            }
        } else {
            // random permutation of 4 values 
            int blocks = 4;
            int count = 0;
            int * narray = (int *) malloc(blocks*sizeof(int));
            while (count < records) {
                for (i = 0; i<blocks; i++)
                    narray[i] = i;
                for (i = 0; i<blocks && count < records; i++) {
                    int key = (int) (drand48() * (blocks - i)) + i;
                    assert(i <= key && key < blocks);
                    int temp = narray[i]; narray[i] = narray[key]; narray[key] = temp;
                    printf("REQ %d\n", narray[i]+1);
                    count++;
                }
            }
            free(narray);
        }
    }
    else {
        printf("geninput has invalid list type: %d\n", list_type);
        exit(3);
    }
    printf("SORT %d\n", sort_type);
    if (records <= 20)
        printf("PRINTREQ\n");
    printf("QUIT\n");
    exit(0);
}

