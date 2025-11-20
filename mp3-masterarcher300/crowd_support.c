/* crowd_support.c 
 * Drew Buley
 * ajbuley
 * C20407096
 * ECE 2230 Fall 2024
 * MP3
 *
 * Purpose: The code in this file calls for operations to be 
 *          made on the fifo and task list independent of their
 *          defined data structure. To do so, it calls functions
 *          from linked_list_lib.c. This file can only access the 
 *          user data contained in the data structure, not
 *          the data structure itself.
 *
 * Assumptions: linked_list_lib.c remains memory safe and its
 *              functions do as expected. 
 *
 * Bugs:
 */
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "linked_list_lib.h"
#include "crowd_support.h"

/* crowd_rank_aps is required by the linked list ADT for sorted lists. 
 *
 * This function returns 
 *     1 if rec_a should be closer to the front than rec_b,
 *    -1 if rec_b is to be considered closer to the front, and 
 *     0 if the records are equal.
 *
 * For the service records we want to sort from smallest to largest department
 * ID.  Closer to the front means a smaller department ID.
 *
 * The function expects pointers to two record structures, and it is an error
 * if either is NULL
 *
 * THIS FUNCTION SHOULD NOT BE CHANGED
 */
int crowd_rank_depts(const service_info_t *record_a, const service_info_t *record_b)
{
    assert(record_a != NULL && record_b !=NULL);

    if (record_a->department_id < record_b->department_id)
        return 1;
    else if (record_a->department_id > record_b->department_id)
        return -1;
    else
        return 0;
}

/* print the records in a list 
 *
 * This function provides an example on how to iterate through
 * the list using the access function.
 */
void crowd_print_list(linked_list_t *list_ptr, const char *type_of_list)
{
    assert(strcmp(type_of_list, "FIFO")==0 || strcmp(type_of_list, "Task")==0);
    service_info_t *rec_ptr;
    int num_in_list = linked_list_count(list_ptr);
    int counter = 0;

    if (num_in_list == 0) {
        printf("%s list is empty\n", type_of_list);
    } else {
        printf("%s list has %d record%s\n", type_of_list, num_in_list, num_in_list==1?"":"s");
        rec_ptr = linked_list_access(list_ptr, counter);
        while (rec_ptr != NULL)
        {
            printf("%d: ", counter);
            crowd_print_service_info(rec_ptr);
            counter++;
            rec_ptr = linked_list_access(list_ptr, counter);
        }
        assert(num_in_list == counter);
    }
    printf("\n");
}

/* This creates the task list for storing service records that are maintained
 * in sorted order based on department id.  There are 
 * no records with duplicate department ids.  
 *
 * Notice that the function pointer for the comparison
 * function is passed in as an agrument.  The comparison function is used 
 * by the sorted insert (and validate) function. 
 *
 */
linked_list_t *crowd_create_tasklist(void)
{
    return linked_list_construct(crowd_rank_depts);
}

/* inserts a service_info_t memory block into the back of the fifo list.  
 *
 * The function first gathers all the service information from the input.
 *
 * Inputs:
 *     fifo: pointer to the list created by the construction function
 *     dept_id: department ID
 */
void crowd_req(linked_list_t *list_ptr, int dept_id)
{
    service_info_t *rec_ptr = crowd_build_service_record(dept_id);
    linked_list_insert(list_ptr, rec_ptr, LLIST_BACK);

    //commented out to reduce cmd line output when running huge list sizes
    //printf("Appended %d to FIFO queue\n", rec_ptr->department_id);
}


/* Sorts the FIFO list using a method selected by the user
 * Will not return an values, but list will be set to sorted state
 *
 * Inputs:
 *     list ptr to the FIFO list
 *     Sorting method chosen
 * Outputs:
 */
void crowd_sort(linked_list_t **fifo, int sort_type) {
    //sets up timing for sorting functions
    clock_t start, end;
    double elapse_time; /* time in milliseconds */
    int initialcount = linked_list_count (*fifo);
    start = clock();

    //calling sort method
    linked_list_sort(fifo, sort_type, crowd_rank_depts);
    end = clock();

    //adjusting time units
    elapse_time = 1000.0 * ((double) (end - start)) / CLOCKS_PER_SEC;
    assert(linked_list_count (*fifo) == initialcount);
    //final time of sort
    printf("%d\t%f\t%d\n", initialcount, elapse_time, sort_type);
}

/* Move as many service request information blocks from the FIFO list into the
 * task list as possible subject to the following rules. The task list is 
 * always maintained such that the list is ordered using the department ID.
 * Lowest ID’s are toward the front of the list, 
 * 
 * The process algorithm first considers the item at the front of the FIFO list.  
 * If the item has a department ID that matches one found in the task list, 
 * then the old service request in the task list is freed and the service memory
 * block replaces the old block.  Clearly, the size of the task list does not 
 * change and no blocks change positions within the task list.  If the service 
 * block is not found in the list, then the process stops if the task list is 
 * full. But, if the task list is not full, then the service block is inserted 
 * into the task list so that the list is in sorted order. The process repeats 
 * until the FIFO list is empty, or the head item in the FIFO list cannot be 
 * inserted into the task list because the task list is full.
 *
 * Note the process stops as soon as the head item in the FIFO list cannot be 
 * moved.  Never skip over the first item in the FIFO list to see if a later 
 * item could replace an item in the task list. 
 *
 * Inputs
 *     fifo_ptr: pointer to the fifo list created by the construction function
 *     task_ptr: pointer to the task list created by the construction function
 */

void crowd_process(linked_list_t *fifo_ptr, linked_list_t *task_ptr, int task_max_size)
{
    
    service_info_t *fifo_head = NULL;
    service_info_t *temp = NULL;
    int pos_index = -2;
    int count = 0;
    //while the fifo list is not empty
    while (linked_list_count(fifo_ptr) != 0) {

        //find record to be processed
        fifo_head = linked_list_access(fifo_ptr, 0);
        if (fifo_head == NULL) {
            break;
        }

        //check if head is duplicant
        temp = linked_list_elem_find(task_ptr, fifo_head, &pos_index);
        if (temp != NULL) {
            //duplicant rec insert
            temp = linked_list_remove(task_ptr, pos_index);
            free(temp);
            fifo_head = linked_list_remove(fifo_ptr, LLIST_FRONT);
            linked_list_insert_sorted(task_ptr, fifo_head);
            fifo_head = NULL;
            count++;
            continue; //duplicant inserted
        }

        //test room to insert non dupe record
        if (linked_list_count(task_ptr) >= task_max_size) {
            break;
        }
        
        //insert the non-dupe record
        temp = linked_list_remove(fifo_ptr, LLIST_FRONT);
        linked_list_insert_sorted(task_ptr, temp);
        count++;
        fifo_head = NULL;
        continue;
    }

    //list is empty so reset these access points
    if (fifo_ptr->llcount == 0) {
        fifo_ptr->llhead = NULL;
        fifo_ptr->lltail = NULL;
    }

    if (count == 0) {
        printf("No service records could be moved to task list \n");
    } else {
        printf("Moved %d service request%s\n", count, count==1?"":"s");
    }
}

/* Search the task list for a service record with the matching department ID.
 * If a match is found, the service record memory block is removed from the list,
 * the ticket number is assigned, and the record is printed, and finally freed 
 * (since we are not implementing the next step of forwarding the record to a 
 * system to assign a technician).  
 *
 * Inputs:
 *     task: pointer to the task list created by the construction function
 *     id: department id that should receive an assignment
 */
void crowd_assign(linked_list_t *task_ptr, int dept_id)
{
    static int next_ticket_num = 0;  // this helps with grading! do not change
    
    int counter = 0;

    service_info_t *service_ptr = linked_list_access(task_ptr, counter);

    //locate matching record
    while (service_ptr != NULL) {
        if (service_ptr->department_id == dept_id) {
            break;
        } else {
            counter++;
            service_ptr = linked_list_access(task_ptr, counter);
        }
    }

    //remove found record from task
    if (service_ptr != NULL) {
        service_ptr = linked_list_remove(task_ptr, counter);
    }


    // if a matching service record is found, then set the ticket number and
    // print the record.
    if (service_ptr != NULL) {
        service_ptr->ticket_number = ++next_ticket_num;  // for grading do not change
        crowd_print_service_info(service_ptr);
        printf("Assigned ticket: %d\n", dept_id);
    } else {
        printf("Assign ticket did not find: %d\n", dept_id);
        return;
    }
    free(service_ptr);
}

/* Search for records in the both lists that match the dept_id given as the input.
 * 
 * For each record found, it is printed.  The item is not removed and the list
 * is not changed.
 * Inputs
 *     fifo_ptr: the fifo list may have multiple matching records.  
 *     task_ptr: the task list can have only one matching record.
 *     id: department ID to search for
 *
 */
void crowd_find(linked_list_t *fifo_ptr, linked_list_t *task_ptr, int dept_id)
{
    service_info_t rec;
    rec.department_id = dept_id;
    
    int found_count = 0;
    int fifo_count = 0;
    int task_count = 0;
    int counter = 0;
    service_info_t *temprec = linked_list_access(fifo_ptr, counter);
    
    printf("Records found on the FIFO list:\n");
    //while fifo has 1+ record
    while (temprec != NULL) {
        if (temprec->department_id == dept_id) {
            fifo_count++;
            crowd_print_service_info(temprec);
        }
        counter++;
        temprec = linked_list_access(fifo_ptr, counter);
    }

    printf("\nRecord found on the task list:\n");
    counter = 0;
    temprec = linked_list_access(task_ptr, counter);
    //while task has 1+ record
    while (temprec != NULL) {
        if (temprec->department_id == dept_id) {
            task_count++;
            crowd_print_service_info(temprec);
        }
        temprec = linked_list_access(task_ptr, ++counter);
    }

    found_count = fifo_count + task_count;

    if (found_count == 0) {
        printf("\nDid not find any service requests from department: %d\n", rec.department_id);
    } else {
        printf("\n%d service request%s found for department: %d\n", 
                found_count, found_count==1?"":"s", dept_id); 
    }
}

/* prints size of each of the two lists
 */
void crowd_stats(linked_list_t *fifo, linked_list_t *task, int task_max_size)
{
    //obtains then prints the count of each list
    int fifo_count = linked_list_count(fifo);
    int task_count = linked_list_count(task);
    printf("Number service records in FIFO list: %d\n", fifo_count);
    printf("Number task tickets in task list: %d, Array size: %d\n", 
            task_count, task_max_size);
}

/* this function frees the memory for either a sorted or unsorted list.
 */
void crowd_cleanup(linked_list_t *list_ptr)
{
    linked_list_destruct(list_ptr);
}

/* Prompts user for information about the service request.
 * The input is not checked for errors but will default to an acceptable value
 * if the input is incorrect or missing.
 *
 * This function creates the memory block for the record.
 *
 * DO NOT CHANGE THIS FUNCTION!
 */
service_info_t *crowd_build_service_record(int dept_id)
{
    //static int order_created = 0;  // commented out for MP3 testing

    /* Commented out as not testing printed output in MP3
    char line[MAXLINE];
    char str[MAXLINE] = "";
    char letter;
    */
    service_info_t *new = (service_info_t *) calloc(1, sizeof(service_info_t));

    assert(new != NULL && "get service info does not have valid record");

    new->department_id = dept_id;
    new->order_received = 0; //order_created++ COMMENTED OUT FOR UNIT TESTING ONLY

    //Members not dept_id set to 0 for improved testing
    //of the sort functionality for MP3

    //printf("user id (string):");
    /*fgets(line, MAXLINE, stdin);
    sscanf(line, "%s", str); // grabs first token on line
    if (strlen(str) > 0 && strlen(str) < USIZE)
        strcpy(new->username, str);
    else
        strcpy(new->username, "unknown");
    
    printf("Dept IP address:");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &new->ip_address);
    printf("Building location code:");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &new->location_code);

    printf("Authenticated (T/F):");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%s", str);
    if (strcmp(str, "T")==0 || strcmp(str, "t")==0)
	new->authenticated = 1;
    else
	new->authenticated = 0;

    printf("Password reset required (Y/N):");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%s", str);
    if (strcmp(str, "Y")==0 || strcmp(str, "y")==0)
	new->password = 1;
    else
	new->password = 0;

    printf("Security key letter (a b e g h n s):");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%c", &letter);
    if (letter < 'a' || letter > 'z')
	letter = 'a';
    new->security_key = letter - 'a';

    printf("Time received (int):");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &new->time_received);
    printf("\n");
    */
    return new;
}

/* print the information for a particular service record 
 *
 * Input is a pointer to a record, and no entries are changed.
 *
 * DO NOT CHANGE THIS FUNCTION!
 */
void crowd_print_service_info(service_info_t *rec)
{
    assert(rec != NULL && "print service info does not have valid record");
    printf(" Dept ID: %d, tk: %d, user %8s, IP: %d, Loc: %d,", rec->department_id, rec->ticket_number,
            rec->username, rec->ip_address, rec->location_code);
    printf(" Auth: %s,", rec->authenticated ? "T" : "F"); 
    printf(" PW: %s,", rec->password ? "Y" : "N"); 
    printf(" Key: %c,", (char) (rec->security_key + 'a'));
    printf(" Time: %d, order: %d\n", rec->time_received, rec->order_received);
}

/* commands specified to vim. ts: tabstop, sts: soft tabstop sw: shiftwidth */
/* vi:set ts=8 sts=4 sw=4 et: */
