/* linked_list_lib.c
 * Drew Buley
 * ajbuley
 * ECE 2230 Fall 2024
 * MP3
 *
 *
 * Assumptions: These functions attempt to safeguard themselves against
 *              scenarios that would cause memory or bad value issues.
 *              However, it is assumed that the function that calls
 *              these function passes only correct values, that it
 *              only calls the function when needed, and deals with the
 *              return values (if any) correctly.
 *
 * Bugs:
 */

#define _GNU_SOURCE //so that stdlib.h defines qsort_r
#include <stdlib.h>
#include <assert.h>

#include "linked_list_lib.h"   // public functions for two-way linked list ADT

#define TRUE  1
#define FALSE 0

// prototypes for private functions used in linked_list.c only
void list_debug_validate(linked_list_t *L);

void insertion_sort(linked_list_t **list_ptr);
void recur_select_sort(linked_list_t *list_ptr, llnode_t *m, llnode_t *n);
void iter_select_sort(linked_list_t *list_ptr);
void merge_sort(linked_list_t *list_ptr);
int my_compare(const void *p_a, const void *p_b, void * lptr);
void qsort_linked(linked_list_t *list_ptr);
linked_list_t *combine(linked_list_t *list_combine, linked_list_t *left, linked_list_t *right);

/* ----- below are the functions  ----- */

/* Allocates a new, empty list 
 *
 * If the comparison function is NULL, then the list is unsorted.
 *
 * Otherwise, the list is initially assumed to be sorted.  Note that if 
 * linked_list_insert is used the list is changed to unsorted.  
 *
 * Use linked_destruct to remove and deallocate all elements on a list 
 * and the header block.
 *
 * (This function is written and no changes needed. It provides an example
 *  of how save the comparison function pointer.  See other examples in this
 *  file.)
 */
linked_list_t * linked_list_construct(int (*compare_function)(const mydata_t *, const mydata_t *))
{
    linked_list_t *L;

    L = (linked_list_t *) malloc(sizeof(linked_list_t));
    L->llhead = NULL;
    L->lltail = NULL;
    L->llcount = 0;
    L->comp_proc = compare_function;
    if (compare_function == NULL)
        L->llsortedstate = FALSE;
    else
        L->llsortedstate = TRUE;

    /* the last line of this function must call validate */
    //list_debug_validate(L);
    return L;
}

/* Deallocates the contents of the specified list, releasing associated memory
 * resources for other purposes.
 *
 * Free all elements in the list and the header block.
 */
void linked_list_destruct(linked_list_t *list_ptr)
{
    /* the first line must validate the list */
    //list_debug_validate(list_ptr);
    
    llnode_t *rover = NULL;
    list_ptr->lltail = NULL;
    while (list_ptr->llcount != 0) {
        rover=list_ptr->llhead->rightlink;
        free(list_ptr->llhead->userdata);
        free(list_ptr->llhead);
        list_ptr->llcount--;
        list_ptr->llhead = rover;
    }
    //no nodes left in list
    free(list_ptr);
    list_ptr = NULL;
}

/* Obtains a pointer to an element stored in the specified list, at the
 * specified list position
 * 
 * list_ptr: pointer to list-of-interest.  A pointer to an empty list is
 *           obtained from linked_list_construct.
 *
 * pos_index: position of the element to be accessed.  Index starts at 0 at
 *            front of the list, and incremented by one until the back is
 *            reached.  Can also specify LLIST_FRONT and LLIST_BACK
 *
 * return value: pointer to the mydata_t element accessed in the list at the
 * index position.  A value NULL is returned if the pos_index does not 
 * correspond to an element in the list.
 */
mydata_t * linked_list_access(linked_list_t *list_ptr, int pos_index)
{
    assert(list_ptr != NULL);
    /* debugging function to verify that the structure of the list is valid */
    //list_debug_validate(list_ptr);

    // the list is empty
    if (list_ptr->llcount == 0) {
        return NULL;
    }
    //accessing the head
    else if (pos_index == LLIST_FRONT || pos_index == 0) {
        return list_ptr->llhead->userdata;
    }
    //accessing the tail
    else if (pos_index == LLIST_BACK || pos_index == list_ptr->llcount - 1) {
        return list_ptr->lltail->userdata;
    }
    //accessing invalid record position
    else if (pos_index < 0 || pos_index >= list_ptr->llcount)
        return NULL;   // does not correspond to position in list

    //accessing record in middle of list
    int current_index = 0;
    llnode_t * rover = list_ptr->llhead;
    while(current_index < pos_index) {
        rover=rover->rightlink;
        current_index++;
    }
    return rover->userdata;
}

/* Finds an element in a list and returns a pointer to the mydata_t memory
 * block.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * elem_ptr: element against which other elements in the list are compared.
 *           Note: it is required that the comp_proc function pointer is used
 *           to check for a match.  It is found in the linked_list_t header block. 
 *
 * NOTICE: pos_index is returned and is not an input value!
 *
 * The function returns a pointer to the mydata_t memory block for the 
 * llnode_t that contains the first matching element if a match if found.  
 * If a match is not found the return value is NULL.
 *
 * The function also returns the integer position of matching element with the
 *           lowest index.  If a matching element is not found, the position
 *           index that is returned should be -1. 
 *
 * pos_index: used as a return value for the position index of matching element
 */
mydata_t * linked_list_elem_find(linked_list_t *list_ptr, mydata_t *elem_ptr, int *pos_index)
{
    //list_debug_validate(list_ptr);

    //list has no nodes
    if (list_ptr->llcount == 0) {
        *pos_index = -1;
        return NULL;
    }
    //list has 1+ nodes
    int index = 0;
    llnode_t *rover = list_ptr->llhead;
    while(rover != NULL) {
        if (rover->userdata == elem_ptr) {
            *pos_index = index;
            return rover->userdata;
        }
        rover = rover->rightlink;
        index++;
    }
    *pos_index = -1;
    return NULL;
}

/* Inserts the data element into the specified list at the specified
 * position.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * elem_ptr: pointer to the memory block to be inserted into list.
 *
 * pos_index: numeric position index of the element to be inserted into the 
 *            list.  Index starts at 0 at front of the list, and incremented by 
 *            one until the back is reached.  The index can also be equal
 *            to LLIST_FRONT or LLIST_BACK (these are special negative 
 *            values use to provide a short cut for adding to the front 
 *            or back of the list).
 *
 * If pos_index is greater than the number of elements currently in the list, 
 * the element is simply appended to the end of the list (no additional elements
 * are inserted).
 *
 * Note that use of this function results in the list to be marked as unsorted,
 * even if the element has been inserted in the correct position.  That is, on
 * completion of this subroutine the list_ptr->llsortedstate must be equal 
 * to FALSE.
 */
void linked_list_insert(linked_list_t *list_ptr, mydata_t *elem_ptr, int pos_index)
{
    assert(list_ptr != NULL);
    assert(pos_index == LLIST_FRONT || pos_index == LLIST_BACK || pos_index >= 0);

    llnode_t *new_node = (llnode_t*)malloc(sizeof(llnode_t));
    new_node->userdata = elem_ptr;
    new_node->rightlink = NULL;
    new_node->leftlink=NULL;
    elem_ptr = NULL;

    //ll empty
    if (list_ptr->llcount == 0) {
        list_ptr->llhead = new_node;
        list_ptr->lltail = new_node;

        list_ptr->llcount++;
        new_node = NULL;
    //new head of ll
    }else if (pos_index == 0 || pos_index == LLIST_FRONT) {
        new_node->rightlink = list_ptr->llhead;
        list_ptr->llhead->leftlink = new_node;
        //update lltag
        list_ptr->llcount++;
        list_ptr->llhead = new_node;
        new_node=NULL;
    
    //new tail of ll
    } else if ((pos_index >= list_ptr->llcount) || pos_index == LLIST_BACK) {
        new_node->leftlink = list_ptr->lltail;
        list_ptr->lltail->rightlink = new_node;

        //update lltag
        list_ptr->llcount++;
        list_ptr->lltail = new_node;
        new_node=NULL;

    //insert at specific position
    } else {
        int current_rec = 0;
        llnode_t * rover = list_ptr->llhead;
        while (current_rec < pos_index) {
            rover = rover->rightlink;
            current_rec++;
        }
        new_node->rightlink = rover;
        new_node->leftlink = rover->leftlink;

        rover->leftlink = new_node;
        new_node->leftlink->rightlink = new_node;
        list_ptr->llcount++;
    }

    /* the last two lines of this function must be the following */
    if (list_ptr->llsortedstate == TRUE) 
	list_ptr->llsortedstate = FALSE;
    //list_debug_validate(list_ptr);
}

void linked_list_sort(linked_list_t **list_ptr, int sort_type, int (*fcomp)(const mydata_t *, const mydata_t *)) {


    (*list_ptr)->comp_proc = fcomp;
    if (sort_type == 1) {
        insertion_sort(list_ptr);
    //might need to add dereference to all these functions...
    } else if (sort_type == 2) {
        recur_select_sort(*list_ptr, (*list_ptr)->llhead, (*list_ptr)->lltail);
    } else if (sort_type == 3) {
        iter_select_sort(*list_ptr);
    } else if (sort_type == 4) {
        merge_sort(*list_ptr);
    } else if (sort_type == 5) {
        qsort_linked(*list_ptr);
    }

    (*list_ptr)->llsortedstate = TRUE;
    list_debug_validate(*list_ptr);
    return;
}

/* This is a private function called by merge_sort.
 * it serves to combine two ascending sorted linked lists into
 * a single list. It is assumed left and right have at least one element in them to start
 * Inputs:
 *      left and right are both pointers to header blocks of sorted linked lists
 * Outputs:
 *      list_combine is a pointer to the newly created merged linked list
*/
linked_list_t *combine(linked_list_t *list_combine, linked_list_t *left, linked_list_t *right) {
    //loop over left and right placing heads in back of storage
    //if one list is empty, place all of other list at back of
    list_combine->llhead = NULL;
    list_combine->lltail = NULL;
    list_combine->llcount = 0;
    int compare_val;
    mydata_t *temp;

    while ((left->llhead != NULL) || (right->llhead != NULL)) {

        //handling when one half runs out of entries before the other
        if (left->llhead == NULL) {
            //add rest of right to list_combine
            right->llhead->leftlink = list_combine->lltail;
            list_combine->lltail->rightlink = right->llhead;
            list_combine->lltail = right->lltail;
            list_combine->llcount += right->llcount;
            break;
        } else if (right->llhead == NULL) {
            //add rest of left to list_combine
            left->llhead->leftlink = list_combine->lltail;
            list_combine->lltail->rightlink = left->llhead;
            list_combine->lltail = left->lltail;
            list_combine->llcount += left->llcount;
            break;
        }

        //handling inserting smallest of the two heads into list_combine
        compare_val = list_combine->comp_proc(left->llhead->userdata, right->llhead->userdata);
        if (compare_val == 1) { //left head has smaller value
            temp = linked_list_remove(left, LLIST_FRONT);
            linked_list_insert(list_combine, temp, LLIST_BACK);
        } else { //right head smaller or of equal value, compare_val <=0
            temp = linked_list_remove(right, LLIST_FRONT);
            linked_list_insert(list_combine, temp, LLIST_BACK);
        }
    }
    return list_combine;
}


/* This is a private function called by linked_list_sort.
 * It sorts the list in ascending order by recursively breaking the provided
 * list into a front half and a back half, then recombining them in sorted order.
 * Inputs: 
 *      This function takes input of a pointer to the list to be sorted
 * Outputs: 
 *      No return value, but list will be in sorted state
*/
void merge_sort(linked_list_t *list_ptr) {

    if (list_ptr->llcount > 1) {
        //storage for new parts of lists
        int i;
        linked_list_t *left = linked_list_construct(list_ptr->comp_proc);
        linked_list_t *right = linked_list_construct(list_ptr->comp_proc);

        //setting heads of left and right
        left->llhead = list_ptr->llhead;
        left->llcount = (list_ptr->llcount)/2;
        right->llhead = list_ptr->llhead;

        for (i = 0; i < (list_ptr->llcount)/2; i++) {
            right->llhead = right->llhead->rightlink;
        }
        //setting headerblocks
        right->llcount = list_ptr->llcount - left->llcount;
        left->lltail = right->llhead->leftlink;
        right->lltail = list_ptr->lltail;

        //breaking bonds between lists
        right->llhead->leftlink->rightlink = NULL;
        right->llhead->leftlink = NULL; //not sure this is needed but good safety net
        
        merge_sort(left);
        merge_sort(right);

        list_ptr = combine(list_ptr, left, right);
        //cleanup memory
        free(right);
        free(left);
    } //else do nothing
}



/* This is a private function called by linked_list_sort. 
 * It sorts the list in ascending order starting at the head.
 * Inputs:
 *      A is a list to be sorted
 * Outputs:
 *      no return value but list will be in sorted state
*/
void iter_select_sort(linked_list_t *A) {
   int minposition, m_pos;
   mydata_t *temp = NULL; 
   llnode_t *rover = NULL;
   llnode_t *min_node = NULL;
   llnode_t *m = A->llhead, *n = A->lltail;

   while (m != n) {
   
      rover = m;
      //linked_list_elem_find(A, m->userdata, &minposition); 
      min_node = m;

      do {
         rover = rover->rightlink;
         //if (A->comp_proc(linked_list_access(A, minposition), rover->userdata) == -1) {
         if (A->comp_proc(min_node->userdata, rover->userdata) == -1) {
             //linked_list_elem_find(A, rover->userdata, &minposition);
             min_node = rover;
         }
      } while (rover->rightlink != NULL);
   
      
      linked_list_elem_find(A, min_node->userdata, &minposition);
      linked_list_elem_find(A, m->userdata, &m_pos);
      if (minposition != m_pos) {
          temp = m->userdata;
          m->userdata = linked_list_remove(A, minposition);
          linked_list_insert(A, temp, minposition);
          n = A->lltail;
      }
      m = m->rightlink;
   }
}


// This is a private function called by recur_select_sort.
// Inputs:
//      A is a header to a two way linked list
//      m is a node in the list
//      n is the tail of the list
// Outputs:
//      This function returns the position of the minumum value 
//      between and including nodes m and n.
int FindMin(linked_list_t *A, llnode_t *m, llnode_t *n)   /* assume m<n */
{
   llnode_t *i = m;      /* i is an index that visits all positions from m to n */
   llnode_t *j = m;     /* j is an index that saves the position of the largest */
   int minpos = -1;
            
                     
   do {
      i = i->rightlink;                      /* advance i to point to next node */
      if (A->comp_proc(i->userdata, j->userdata) == 1) {          /* if i < smallest previous j then */
         j = i;              /* save the position, i, of the new smallest in j */
      }
   } while (i != n);           /* stop when all i in m:n have been tested */

   linked_list_elem_find(A, j->userdata, &minpos);
   return minpos;  // return index of min record in range m:n
}

/* This function recursively calls itself to sort the provided list by 
 * swapping the node with the min value between, and including, m and n
 * with the node at m.
 * Inputs:
 *      linked list tag containing information about the list
 *      m and n are node pointers that will be the bounds of the sorted section
 *      for this project, they will be the head and tail of the list
 * Outputs:
 *      does not return anything but list will be sorted
 */
void recur_select_sort(linked_list_t *A, llnode_t *m, llnode_t *n)
{
   int MinPosition, m_pos;   /* MinPosition is the location of A's smallest item  */
   mydata_t *temp;                     /* temp is used to exchange items in A */
   

   if ((m != n) && (m != NULL)){               /* if there is more than one number to sort */
   
      /* Let MinPosition be the index of the smallest number in the list A */
         MinPosition = FindMin(A,m,n);
      
      /* Exchange rec of m with record of MinPosition */
         linked_list_elem_find(A, m->userdata, &m_pos);
         if (MinPosition != m_pos) {
             temp = m->userdata;
             m->userdata = linked_list_remove(A, MinPosition);

             linked_list_insert(A, temp, MinPosition);
             n = A->lltail;
         }
         /* recur_select_sort with m one deeper into the list */
         recur_select_sort(A, m->rightlink, n);
   }
}

/* This is a private function used by linked_list_sort()
 * it takes input of a list and uses the insertion sort method
 * to sort the list. 
 *
 * Inputs:
 *      list_ptr is a pointer to the list that will be sorted.
 * Outputs:
 *      No output returned but the list's data will have been sorted
 */
void insertion_sort(linked_list_t **list_ptr) {

    //create new space for priority list
    linked_list_t *priority = linked_list_construct((*list_ptr)->comp_proc);
    mydata_t *temp = NULL;
    priority->llsortedstate = TRUE;

    // sort elements into new list
    while ((*list_ptr)->llhead != NULL) {
        temp = linked_list_remove(*list_ptr, 0);
        linked_list_insert_sorted(priority, temp);
    }
    //fully delete old list and change address of list_ptr to new list
    free(*list_ptr);
    *list_ptr = priority;
}


/* this is a comparison function used for qsort adapted for a two way linked list
 * Inputs:
 *      pointers to user records a and b
 *      pointer to linked_list header block that contains project specific userdata comparison
 * Outputs:
 *      returns 1 if record a should go before record b
 *      returns -1 if record b should go before record a
 *      returns 0 if records are equal in sequencing
 */
int my_compare(const void *p_a, const void *p_b, void * lptr)
{
    linked_list_t *list_ptr = (linked_list_t *) lptr;
    return list_ptr->comp_proc (*(mydata_t **) p_b, *(mydata_t **) p_a);
}

void qsort_linked(linked_list_t *list_ptr) {
    //sort type is adapted qsort
    int i, Asize = linked_list_count(list_ptr);
    assert((list_ptr)->comp_proc != NULL);
    mydata_t ** QsortA = (mydata_t **) malloc(Asize*sizeof(mydata_t *));
    for (i = 0; i < Asize; i++) {
    QsortA[i] = linked_list_remove(list_ptr, LLIST_FRONT);
    }
    //calling sort on array of pointers to userdata records
    qsort_r(QsortA, Asize, sizeof(mydata_t *), my_compare, list_ptr);
    for (i = 0; i < Asize; i++) {
    linked_list_insert(list_ptr, QsortA[i], LLIST_BACK);
    }
    free(QsortA);
}


/* Inserts the element into the specified sorted list at the proper position,
 * as defined by the comp_proc.  This function is defined in the header:
 *     list_ptr->comp_proc(A, B)
 *
 * The comparison procedure must accept two arguments (A and B) which are both
 * pointers to elements of type mydata_t.  The comparison procedure returns an
 * integer code which indicates the precedence relationship between the two
 * elements.  The integer code takes on the following values:
 *    1: A should be closer to the front of the list than B
 *   -1: B should be closer to the front of the list than A
 *    0: A and B are equal in rank
 *
 * If the element to be inserted is equal in rank to an element already
 * in the list, the newly inserted element will be placed _after all_ the
 * elements of equal rank that are already in the list.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * elem_ptr: pointer to the element to be inserted into list.
 *
 * If you use linked_list_insert_sorted, the list preserves its sorted nature.
 *
 * If you use linked_list_insert, the list will be considered to be unsorted, even
 * if the element has been inserted in the correct position.
 *
 * If the list is not sorted and you call linked_list_insert_sorted, this subroutine
 * must generate a system error and the program should immediately stop.
 *
 */
void linked_list_insert_sorted(linked_list_t *list_ptr, mydata_t *elem_ptr)
{
    assert(list_ptr != NULL);
    assert(list_ptr->llsortedstate == TRUE);

    llnode_t *new_node = (llnode_t *)malloc(sizeof(llnode_t));
    new_node->userdata = elem_ptr;
    new_node->rightlink = NULL;
    new_node->leftlink = NULL;
    elem_ptr = NULL;

    //empty list insert
    if (list_ptr->llcount == 0) { 
        list_ptr->llhead = new_node;
        list_ptr->lltail = new_node;
        new_node = NULL;
        list_ptr->llcount++;
    //new head of ll
    } //else if (list_ptr->llhead->userdata->department_id > new_node->userdata->department_id) {
      else if (list_ptr->comp_proc(new_node->userdata, list_ptr->llhead->userdata) == 1) {
        list_ptr->llhead->leftlink = new_node;
        new_node->rightlink = list_ptr->llhead;
        list_ptr->llcount++;
        list_ptr->llhead = new_node;
        new_node = NULL; 
    } else {
        //insert in middle of linked list
        llnode_t *rover = list_ptr->llhead;
        while (rover->rightlink != NULL) {
            //if (rover->rightlink->userdata->department_id > new_node->userdata->department_id) {
            //is True when new rec id is less than rover->rightlink's or when they are equal
            if (list_ptr->comp_proc(rover->rightlink->userdata, new_node->userdata) <= 0) {
                new_node->leftlink = rover;
                new_node->rightlink = rover->rightlink;
                rover->rightlink->leftlink = new_node;
                rover->rightlink = new_node;

                list_ptr->llcount++;
                new_node = NULL;
                break;
            } else {
                rover = rover->rightlink;
            }
        }
        //new tail of linked list
        if (rover->rightlink == NULL) {
            new_node->leftlink = rover;
            rover->rightlink = new_node;
            list_ptr->lltail = new_node;
            list_ptr->llcount++;
            new_node = NULL;
        }
    }
                

    /* the last line of this function must be the following */
    //list_debug_validate(list_ptr);
}

/* Removes the element from the specified list that is found at the 
 * specified list position.  A pointer to the data element is returned.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * pos_index: position of the element to be removed.  Index starts at 0 at
 *            front of the list, and incremented by one until the back is
 *            reached.  Can also specify LLIST_FRONT and LLIST_BACK
 *
 * Attempting to remove an element at a position index that is not contained in
 * the list will result in no element being removed, and a NULL pointer will be
 * returned.
 */
mydata_t * linked_list_remove(linked_list_t *list_ptr, int pos_index)
{
    assert(list_ptr != NULL);
    assert(pos_index == LLIST_FRONT || pos_index == LLIST_BACK || pos_index >= 0);

    //empty list received
    if (list_ptr->llcount == 0) {
        list_debug_validate(list_ptr);
        return NULL;
    }

    mydata_t *olddata = NULL; //will be returned to allow use before freeing it

    //removing head of ll
    if ((pos_index == 0) || (pos_index == LLIST_FRONT)) {
        olddata = list_ptr->llhead->userdata;

        if (list_ptr->llcount == 1) {
            list_ptr->lltail = NULL;
            free(list_ptr->llhead);
            list_ptr->llhead = NULL;
        } else {
            list_ptr->llhead = list_ptr->llhead->rightlink;
            free(list_ptr->llhead->leftlink);
            list_ptr->llhead->leftlink = NULL;
        }
        list_ptr->llcount--;
        //list_debug_validate(list_ptr);
        return olddata;
    }

    //removing tail of ll
    if ((pos_index == ((list_ptr->llcount) - 1)) || (pos_index == LLIST_BACK)) {
        olddata = list_ptr->lltail->userdata;

        if (list_ptr->llcount == 1) {
            list_ptr->llhead = NULL;
            free(list_ptr->lltail);
            list_ptr->lltail = NULL;
        } else {
            list_ptr->lltail = list_ptr->lltail->leftlink;
            free(list_ptr->lltail->rightlink);
            list_ptr->lltail->rightlink = NULL;
        }
        list_ptr->llcount--;
        //list_debug_validate(list_ptr);
        return olddata;
    }

    int current_rec = 0;
    llnode_t *rover = list_ptr->llhead;

    //removing record in middle of linked list
    while (rover != NULL) {
       if (current_rec == pos_index) {
           rover->rightlink->leftlink = rover->leftlink;
           rover->leftlink->rightlink = rover->rightlink;

           list_ptr->llcount--;
           olddata = rover->userdata;
           rover->userdata = NULL;
           free(rover);
           break;
       } else {
           current_rec++;
           rover = rover->rightlink;
       }
    }
    //list_debug_validate(list_ptr);
    return olddata;


    /* the last line should verify the list is valid after the remove */
}

/* Obtains the length of the specified list, that is, the number of elements
 * that the list contains.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * Returns an integer equal to the number of elements stored in the list.  An
 * empty list has a size of zero.
 *
 * (This function is already written, so no changes necessary.)
 */
int linked_list_count(linked_list_t *list_ptr)
{
    assert(list_ptr != NULL);
    assert(list_ptr->llcount >= 0);
    return list_ptr->llcount;
}


/* This function verifies that the pointers for the two-way linked list are
 * valid, and that the list size matches the number of items in the list.
 *
 * If the linked list is sorted it also checks that the elements in the list
 * appear in the proper order.
 *
 * The function produces no output if the two-way linked list is correct.  It
 * causes the program to terminate and print a line beginning with "Assertion
 * failed:" if an error is detected.
 *
 * The checks are not exhaustive, so an error may still exist in the
 * list even if these checks pass.
 *
 * YOU MUST NOT CHANGE THIS FUNCTION.  WE USE IT DURING GRADING TO VERIFY THAT
 * YOUR LIST IS CONSISTENT.
 */
void list_debug_validate(linked_list_t *L)
{
    llnode_t *N;
    int count = 0;
    assert(NULL != L); 
    if (NULL == L->lltail) 
	assert(NULL == L->llhead && 0 == L->llcount);
    else
	assert(NULL == L->lltail->rightlink);
    if (L->llhead != NULL) 
	assert(L->llhead->leftlink == NULL);
    else
	assert(L->lltail == NULL && L->llcount == 0);
    if (0 == L->llcount) assert(NULL == L->llhead && NULL == L->lltail);
    if (L->llhead == L->lltail && L->lltail != NULL) assert(L->llcount == 1);
    if (1 == L->llcount) {
        assert(L->llhead == L->lltail && L->lltail != NULL);
        assert(NULL == L->lltail->rightlink && NULL == L->lltail->leftlink);
        assert(NULL != L->lltail->userdata);
    }
    assert(L->llsortedstate == TRUE || L->llsortedstate == FALSE);
    if (1 < L->llcount) {
        assert(L->llhead != L->lltail && NULL != L->llhead && NULL != L->lltail);
        N = L->lltail;
        while (N != NULL) {
            assert(NULL != N->userdata);
            if (NULL != N->leftlink) assert(N->leftlink->rightlink == N);
            else assert(N == L->llhead);
            count++;
            N = N->leftlink;
        }
        assert(count == L->llcount);
    }
    if (L->llsortedstate && NULL != L->llhead) {
        N = L->llhead;
        while (N->rightlink != NULL) {
            assert(L->comp_proc(N->userdata, N->rightlink->userdata) != -1);   // A <= B or A !> B
            N = N->rightlink;
        }
    }
}
/* commands for vim. ts: tabstop, sts: softtabstop sw: shiftwidth */
/* vi:set ts=8 sts=4 sw=4 et: */

