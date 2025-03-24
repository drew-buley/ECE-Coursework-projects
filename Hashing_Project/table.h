/* table.h 
 * Lab6: Hash Tables 
 * ECE 2230, Fall 2024
 * Drew Buley
 * C20407096
 * 12/5/2024
 */

/* constants used to indicate type of probing.  */
enum ProbeDec_t {LINEAR, DOUBLE, QUAD};

typedef void *data_t;   /* pointer to the information, I, to be stored in the table */
typedef int hashkey_t;   /* the key, K, for the pair (K, I) */

typedef struct table_etag {
    hashkey_t key;
    data_t data_ptr;
} table_entry_t;

typedef struct table_tag {
    // you need to fill in details, and you can change the names!
    int table_size;
    int type_of_probing;
    int num_keys;
    int num_probes;
    table_entry_t *oa;
} table_t;

/*  The empty table is created.  The table must be dynamically allocated and
 *  have a total size of table_size.  The maximum number of (K, I) entries
 *  that can be stored in the table is table_size-1.  For open addressing, 
 *  the table is filled with a special empty key distinct from all other 
 *  nonempty keys.  
 *
 *  The probe_type must be one of {LINEAR, DOUBLE, QUAD}
 *
 *  Do not "correct" the table_size or probe decrement if there is a chance
 *  that the combinaion of table size or probe decrement will not cover
 *  all entries in the table.  Instead we will experiment to determine under
 *  what conditions an incorrect choice of table size and probe decrement
 *  results in poor performance.
 */
table_t *table_construct(int table_size, int probe_type);  

/* Sequentially remove each table entry (K, I) and insert into a new
 * empty table with size new_table_size.  Free the memory for the old table
 * and return the pointer to the new table.  The probe type should remain
 * the same.
 *
 * Do not rehash the table during an insert or delete function call.  Instead
 * use drivers to verify under what conditions rehashing is required, and
 * call the rehash function in the driver to show how the performance
 * can be improved.
 */
table_t *table_rehash(table_t * T, int new_table_size);  

/* returns number of entries in the table */
int table_entries(table_t *);

/* returns 1 if table is full and 0 if not full. */
int table_full(table_t *);

/* returns the number of table entries marked as deleted */
int table_deletekeys(table_t *);
   
/* Insert a new table entry (K, I) into the table provided the table is not
 * already full.  
 * Return:
 *      0 if (K, I) is inserted, 
 *      1 if an older (K, I) is already in the table (update with the new I), or 
 *     -1 if the (K, I) pair cannot be inserted.
 *
 *  You must develop this function using the algorithm from our textbook and 
 *  lecture notes.  The probe must decrement, not increment.
 */
int table_insert(table_t *, hashkey_t K, data_t I);

/* Delete the table entry (K, I) from the table.  
 * Return:
 *     pointer to I, or
 *     null if (K, I) is not found in the table.  
 *
 * See the note on page 490 in Standish's book about marking table entries for
 * deletions when using open addressing.
 */
data_t table_delete(table_t *, hashkey_t K); 

/* Given a key, K, retrieve the pointer to the information, I, from the table,
 * but do not remove (K, I) from the table.  Return NULL if the key is not
 * found.
 *
 *  You must develop this function using the algorithm from our textbook and 
 *  lecture notes.  The probe must decrement, not increment.
 */
data_t table_retrieve(table_t *, hashkey_t K); 

/* Free all information in the table, the table itself, and any additional
 * headers or other supporting data structures.  
 */
void table_destruct(table_t *);

/* The number of probes for the most recent call to table_retrieve,
 * table_insert, or table_delete 
 */
int table_stats(table_t *);  

/* This function is for testing purposes only.  Given an index position into
 * the hash table return the value of the key if data is stored in this 
 * index position.  If the index position does not contain data, then the
 * return value must be INT_MAX.  Make the first
 * lines of this function 
 *       assert(0 <= index && index < table_size); 
 */
hashkey_t table_peek(table_t *T, int index); 

/* Print the table position and keys in a easily readable and compact format.
 * Also, show if an index is marked as empty or deleted.
 * Only useful when the table is small.
 */
void table_debug_print(table_t *T);
