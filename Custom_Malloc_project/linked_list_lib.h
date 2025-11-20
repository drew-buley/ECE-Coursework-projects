/* linked_list_lib.h 
 * Drew Buley
 * ajbuley
 * C20407096
 * ECE 2230 Fall 2024
 * MP4
 *
 * Public functions for two-way linked list
 *
 */

#include "datatypes.h"   // defines mydata_t 

#define LLIST_FRONT -2024
#define LLIST_BACK  -9149403

typedef struct llnode_tag {
    // linked_list_lib.c private members 
    mydata_t *userdata;
    struct llnode_tag *leftlink;
    struct llnode_tag *rightlink;
} llnode_t;

typedef struct linked_list_tag {
    // linked_list_lib.c private members
    llnode_t *llhead;
    llnode_t *lltail;
    int llcount;
    int llsortedstate;
    // linked_list_lib.c private procedure for sorted insert 
    int (*comp_proc)(const mydata_t *, const mydata_t *);
} linked_list_t;

/* public prototype definitions */

/* build and cleanup lists */
mydata_t *     linked_list_access(linked_list_t *list_ptr, int pos_index);
linked_list_t *linked_list_construct(int (*fcomp)(const mydata_t *, const mydata_t *));
int            linked_list_count(linked_list_t *list_ptr);
void           linked_list_destruct(linked_list_t *list_ptr);
mydata_t *     linked_list_elem_find(linked_list_t *list_ptr, mydata_t *elem_ptr, int *pos);
void           linked_list_insert(linked_list_t *list_ptr, mydata_t *elem_ptr, int pos_index);
void           linked_list_insert_sorted(linked_list_t *list_ptr, mydata_t *elem_ptr);
mydata_t *     linked_list_remove(linked_list_t *list_ptr, int pos_index);
void           linked_list_sort(linked_list_t **list_ptr, int sort_type, int (*fcomp)(const mydata_t *, const mydata_t *));

/* commands for vim. ts: tabstop, sts: soft tabstop sw: shiftwidth */
/* vi:set ts=8 sts=4 sw=4 et: */
