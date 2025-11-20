/* C Programs from Chapter 5 of
 * Data Structures, Algorithms and Software Principles in C
 * by Thomas A. Standish
 * Copyright (C) 1995, Addison-Wesley, Inc., all rights reserved
 */


typedef    int    InputArray[6];

int FindMax(InputArray A, int m, int n)   /* assume m<n */
{
   int i = m;      /* i is an index that visits all positions from m to n */
   int j = m;     /* j is an index that saves the position of the largest */
            
                     
   do {
      i++;                      /* advance i to point to next number A[i] */
      if (A[i] > A[j]) {          /* if A[i] > largest previous A[j] then */
         j = i;              /* save the position, i, of the largest in j */
      }
   } while (i != n);           /* stop when all i in m:n have been tested */

   return j;                       /* return j == position of the largest */
}                                                /* number A[j] in A[m:n] */


void SelectionSort(InputArray A, int m, int n)
{
   int   MaxPosition;    /* MaxPosition is the index of A's biggest item  */
   int   temp;                     /* temp is used to exchange items in A */

   if (m < n) {               /* if there is more than one number to sort */
   
      /* Let MaxPosition be the index of the largest number in A[m:n] */
         MaxPosition = FindMax(A,m,n);
      
      /* Exchange A[m] <--> A[MaxPosition] */
         temp = A[m];
         A[m] = A[MaxPosition];
         A[MaxPosition] = temp;
      
      /* SelectionSort the subarray A[m+1:n] */
         SelectionSort(A, m+1, n);
   }
}


void IterativeSelectionSort(InputArray A, int m, int n)
{
   int MaxPosition, temp, i;

   while (m < n) {
   
      i = m; 
      MaxPosition = m;

      do {
         i++;
         if ( A[i] > A[MaxPosition] ) MaxPosition = i;
      } while (i != n);
   
      temp = A[m]; A[m] = A[MaxPosition]; A[MaxPosition] = temp;
   
      m++;
   }
}

/*
void MergeSort(ListType List)
{
   if (the List has more than one item in it) {
      (break the List into two half-lists, L = LeftList and R = RightList)
      (sort the LeftList using MergeSort(L))
      (sort the RightList using MergeSort(R))
      (merge L and R into a single sorted List)
   } else {
      (do nothing, since the list is already sorted)
   }
}
*/

// Bonus content.  Here is how to use qsort_r

/* Comparison function for qsort_r
 *
 * The Linux man page for qsort gives details on requirements for
 * the comparison function.  The thrid argument, lptr, is a pointer
 * to a header block with details needed for the comparison.  
 *
 * For our program, the header has the function pointer for the 
 * comparison function selected for this sort operation.
 *
 * Notice we must cast the pointers because the prototypes use
 * void for all pointers.
 * */
int qsort_compare(const void *p_a, const void *p_b, void * lptr)
{
    linked_list_t *list_ptr = (linked_list_t *) lptr;
    return list_ptr->comp_proc (*(mydata_t **) p_b, *(mydata_t **) p_a);
}

/* A wrapper function to set up the call to qsort_r for our linked list ADT.
 *
 * qsort can only sort arrays.  The array contains links to the
 * userdata.  The comparison function for qsort is given pointers
 * to array positions.  It must dereference the pointer to get the link
 * to the memory block. So, the comparison function must accept a
 * double pointer.  The qsort comparison function also needs list_ptr to
 * find the function pointer for the userdata comparison.
 *
 * Note to use qsort_r, must #define _GNU_SOURCE as the first line in the file.
 * That is, before all include statements. 
 *
 * Assumption:
 *    list_ptr->comp_proc is the function pointer to compare userdata.
 */
void qsort_linked_list(linked_list_t *list_ptr) 
{
    int i, Asize = linked_list_count(list_ptr);
    assert(list_ptr->comp_proc != NULL);
    mydata_t ** QsortA = (mydata_t **) malloc(Asize*sizeof(mydata_t *));

    for (i = 0; i < Asize; i++) {
        QsortA[i] = linked_list_remove(list_ptr, LLIST_FRONT);
    }

    qsort_r(QsortA, Asize, sizeof(mydata_t *), qsort_compare, list_ptr);

    for (i = 0; i < Asize; i++) {
        linked_list_insert(list_ptr, QsortA[i], LLIST_BACK);
    }
    free(QsortA);
}
