/*
 * Drew Buley
 * C20407096
 * 12/05/2024
 * MP6
 *
 * Purpose: This file contains functions for creating and maintaining
 *          a hash table ADT. It was built to be called by lab6.c and was only tested
 *          for functionality related to the drivers contained therein.
 *
 * Assumptions: lab6.c only calls these functions with valid parameters and 
 *              is setup to correctly process the return values of these functions
 *
 * Bugs: Currently does not check table size vs probe type to ensure they are compatible.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <math.h>

#include "table.h"
#include "hashes.h"
#define empty (INT_MAX-1)
#define deleted (INT_MIN+1)

/* This function creates a table ADT that is used in later functions in this file
 * The header stores information about the ADT that other functions will call on
 * such as the number of keys in the table or number of recent probes used
 *
 * Inputs: table size - size of the table to create
           probe type - stored in member of the ADT header for later use
 * 
 * Outputs: pointer to the created table header (which contains a pointer to the 
 *          table ADT as a member)
 */
table_t *table_construct(int table_size, int probe_type) 
{
    assert(table_size > 0);
    // create new table header
    table_t *new_table = (table_t*)malloc(sizeof(table_t));

    //initialize table values
    new_table->table_size = table_size;
    new_table->type_of_probing = probe_type;

    //protection agaist mismatched table sizes and probing styles
    //assignment specs call for this to be disabled
    /*
    if (probe_type == DOUBLE) {
        if ((table_size > 2) && ((table_size % 2)) == 0) {
            //all even numbers larger than 2 are guranteed not to be prime
            printf("Cannot use DOUBLE PROBING with this table size\n");
            printf("Table sizes for DOUBLE must be relatively prime, you entered: %d\n",table_size);
            exit(1);
        }
    } else if (probe_type == QUAD) {
        //fully check if table size is power of 2
        if ((table_size & (table_size - 1)) == 0) {
            printf("Cannot use QUADRATIC PROBING with this table size\n");
            printf("Table sizes for QUAD must be a power of 2, you entered: %d\n", table_size);
            exit(1);
        }
    }
    */
    new_table->num_keys = 0;
    new_table->num_probes = 0;

    //set table keys to default value
    new_table->oa = (table_entry_t *)malloc(new_table->table_size * sizeof(table_entry_t));
    for (int i = 0; i < new_table->table_size; i++) {
        new_table->oa[i].key = empty;
    }
    return new_table;
}

 /* Returns the number of entries in the table ADT
 *
 * Inputs: pointer to the table ADT
 * Outputs: integer of the number of keys in the table
 */
int table_entries(table_t * table) 
{
    return table->num_keys;
}

/* This function returns 1 if the table is full and 0 if it is not full
 * Inputs: pointer to the table
 * Outputs: 0 or 1 depending on conditional
 */
int table_full(table_t * table) 
{
    if (table->table_size - table->num_keys >= 2) {
        return 0;
    } else if (table->table_size - table->num_keys == 1) {
        return 1;
    } else {
        assert(false); //table is overfull
    }
}

/* This function inserts a new entry (K, I) into the table if there is room available
 *
 * Inputs: - pointer to the table ADT
 *         - key value of the new entry
 *         - data associated with that key
 *
 * Outputs:  0 if (K, I) is inserted into the table
 *           1 if was already in table and data (I) was updated to new I
 *           -1 if (K, I) could not be inserted into the table
 */
int table_insert(table_t *table, hashkey_t K, data_t I)
{
    table->num_probes = 0;

    int index = hashes_table_pos(K, table->table_size);
    int prob_dec;

    if (table->type_of_probing == LINEAR) {
        prob_dec = 1;
    } else if (table->type_of_probing == DOUBLE) {
        prob_dec = hashes_probe_dec(K, table->table_size);
    } else {
        assert(table->type_of_probing == QUAD);
        prob_dec = 0;
    }

    int del_found = 0;
    int del_index = -1; // also used as stop condition when no empty slots left in table
    table->num_probes++; //must increment here or insert direct to empty slot will be wrong

    // Find slot to enter (K, I)
    while ((table->oa[index].key != empty) && index != del_index) {
        if (table->oa[index].key == K) {
            free(table->oa[index].data_ptr);
            table->oa[index].data_ptr = I;
            return 1; //replaced data at target
        } else if ((table->oa[index].key == deleted) && (del_found == 0)) {
            //insert here unless find key already in table
            del_index = index;
            del_found++;
        }

        //need to probe additional spot
        if (table->type_of_probing == QUAD) {
            prob_dec++;
        }
        index -= prob_dec;
        while (index < 0) {
            index += table->table_size;
        }
        if (index != del_index) {
            table->num_probes++;
        }
    }

    //check if table full here as could have found dupe to update in a full table in above loop
    if ((table->table_size - table->num_keys) == 1) {
        return -1; //not able to insert into table
    }
    if (del_index != -1) {
        table->oa[del_index].key = K;
        table->oa[del_index].data_ptr = I;
    } else {
        table->oa[index].key = K;
        table->oa[index].data_ptr = I;
    }
    table->num_keys++;
    return 0; //new key inserted
}

data_t table_delete(table_t *table, hashkey_t K) 
{
    int index = hashes_table_pos(K, table->table_size);
    int prob_dec;
    table->num_probes = 1;

    if (table->type_of_probing == LINEAR) {
        prob_dec = 1;
    } else if (table->type_of_probing == DOUBLE) {
        prob_dec = hashes_probe_dec(K, table->table_size);
    } else {
        assert(table->type_of_probing == QUAD);
        prob_dec = 0;
    }

    int init_index = index; //used as stop con when table has no empty cells
    while ((table->oa[index].key != empty)) {
        if (table->oa[index].key == K) {
            //found key to delete
            table->oa[index].key = deleted;
            table->num_keys--;
            return table->oa[index].data_ptr;
        }
        // probe next potential spot
        if (table->type_of_probing == QUAD) {
            prob_dec++;
        }
        index -= prob_dec;
        while (index < 0) {
            index += table->table_size;
        }
        if (index == init_index) { //checks if next index is where loop started
            break;
        }
        table->num_probes++;
    }
    //return null as encountered an empty cell before target key
    return NULL;
}

/* Search table ADT for target key and return data at that key.
 * Inputs: pointer to table ADT
 *         target key to search for
 * Outputs: pointer to data at target key
 *          NULL if key was not found in table
 */
data_t table_retrieve(table_t * table, hashkey_t K) 
{
    int index = hashes_table_pos(K, table->table_size);
    int prob_dec;
    table->num_probes = 1;

    if (table->type_of_probing == LINEAR) {
        prob_dec = 1;
    } else if (table->type_of_probing == DOUBLE) {
        prob_dec = hashes_probe_dec(K, table->table_size);
    } else {
        assert(table->type_of_probing == QUAD);
        prob_dec = 0;
    }

    int init_index = index;
    while((table->oa[index].key != empty)) {
        if (table->oa[index].key == K) {
            //found the key to retrieve
            return table->oa[index].data_ptr;
        }
        //probe next potential location
        if (table->type_of_probing == QUAD) {
            prob_dec++;
        }

        index -= prob_dec;
        while (index < 0) {
            index += table->table_size;
        }
        if (index == init_index) {
            break;
        }
        table->num_probes++;
    }
    //encountered empty cell before target so key not in table
    //or looked through entire table
    return NULL;
}

/* This function rehashes a table ADT. To do this, we construct a new table,
 * copy valid values between them, then free the old table
 * Inputs: pointer to the old table
 *         size of the new table
 * Outputs: pointer to the rehashed table
 */
table_t *table_rehash(table_t * T, int new_table_size) 
{
    table_t *new_table = table_construct(new_table_size, T->type_of_probing);

    for (int i = 0; i < T->table_size; i++) {
        if ((T->oa[i].key == empty) || (T->oa[i].key == deleted)) {
            continue; //don't need to transfer this cell
        }
        int check_ins = table_insert(new_table, T->oa[i].key, T->oa[i].data_ptr);
        assert(check_ins == 0);
        T->num_keys--;
        if (T->num_keys == 0) { //no vaild keys remianing in old table
            break;
        }
    }
    table_destruct(T);
    return new_table;
}

/* This function determines the number of entries in a table that are marked as deleted
 * Inputs: pointer to the table ADT
 * Outputs: Number of entries marked as deleted
 */
int table_deletekeys(table_t *table)
{
    int num_del = 0;
    for (int i = 0; i < table->table_size; i++) {
        if (table->oa[i].key == deleted) {
            num_del++;
        }
    }
    return num_del;
}

/* This function frees all memory from the ADT
 * this includes, the data from each entry, the table itself, and the table header
 * Inputs: table pointer
 * Outputs: None
 */
void table_destruct(table_t * table) 
{
    for (int i = 0 ; i < table->table_size ; i++) {
        if (table->num_keys == 0) { //no dynamic data left to free
            break;
        }
        if (table->oa[i].key != empty && table->oa[i].key != deleted) {
            //found table entry to clear
            free(table->oa[i].data_ptr);
            table->num_keys--;
        }
    }
    //just ADT structures left to fill
    assert(table->num_keys == 0);
    free(table->oa);
    free(table);
}



// ------------------- Debugging Functions----------
/* This function returns the number of probes used in the last
 * insertion, delete, or rehash
 * Inputs: pointer to the table
 * Outputs: number of probes
 */
int table_stats(table_t * table) 
{
    return table->num_probes;
}

/* This function determines the key value at a given index
 * Inputs: pointer to the table header
 * Outputs: key value if data was found
 *          INT_MAX if cell was empty or marked as deleted
 */
hashkey_t table_peek(table_t *table, int index) 
{
    assert(0 <= index && index < table->table_size);
    if (table->oa[index].key == empty || table->oa[index].key == deleted) {
        return INT_MAX;
    }
    return table->oa[index].key;
}

/* Prints the keys at each index for the provided table
 * Inputs: pointer to the table header
 * Outputs: none, directly prints in termina
 */
void table_debug_print(table_t *table) 
{
    //not the prettiest print but is functional as a debugging tool
    printf("\nprinting table of size %d with %d unique keys:\n", table->table_size, table->num_keys);
    printf("index\t\t\t\tkey value\n");
    for (int i = 0; i < table->table_size; i++) {
        if (table->oa[i].key == empty) {
            printf("%d\t\t\t\tempty\n", i);
        } else if (table->oa[i].key == deleted) {
            printf("%d\t\t\t\tdeleted\n", i);
        } else {
            printf("%d\t\t\t\t%d\n", i, table->oa[i].key);
        }
    }
    printf("print completed\n\n");
}



