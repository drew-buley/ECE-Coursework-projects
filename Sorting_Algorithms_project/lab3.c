/* lab3.c
 * Drew Buley
 * ajbuley
 * C20407096
 * ECE 2230 Fall 2024
 * MP3
 *
 * Purpose: Driver file for code found in linked_list.c and crowd_support.c.
 *          The user commands in this file allow the user to perform a variety
 *          of tasks related to managing support service records in two different lists.
 *
 * Assumptions: The main function collects input commands and
 *              calls the appropriate crowd functions. Additionally
 *              invalid inputs assumed to be caught and dealt with accordingly.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "linked_list_lib.h"
#include "crowd_support.h"

int main(int argc, char * argv[])
{
    linked_list_t *fifo = NULL;
    linked_list_t *task = NULL;
    char line[MAXLINE];
    char command[MAXLINE];
    char junk[MAXLINE];
    int task_max_size = -1;
    int token_count;
    int dept_id;

    if (argc != 1) {
        //printf("Error with input paramters.  Usage: ./lab3\n");
        exit(1);
    }

    /*
    printf("lab3 commands:\n");
    printf("fifo : REQ dept-id; PROCESS; PRINTREQ; SORT int\n");
    printf("task : ASSIGNTIC dept-id; PRINTTIC\n");
    printf("     : FINDALL dept-id; STATS; QUIT\n\n");
    
    printf("What is the maximum number of service requests on the task list? ");
    */
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &task_max_size);
    if (task_max_size < 2) {
        //printf("Invalid list size %d input was %s\n", task_max_size, line);
        exit(1);
    } else {
        //printf("task list size: %d\n", task_max_size);
    }

    /* the fifo list is unsorted and the size of the list is not limited */
    fifo = linked_list_construct(NULL);

    /* the task list is sorted and the list size is limited */
    task = crowd_create_tasklist();

    assert(fifo != NULL && "failed to construct the fifo list");
    assert(task != NULL && "failed to construct the task list");

    /* remember fgets includes newline \n unless line too long */
    // excessive prints commented out to reduce chatter
    while (fgets(line, MAXLINE, stdin) != NULL) {
        token_count = sscanf(line, "%s%d%s", command, &dept_id, junk);
        if (token_count == 2 && strcmp(command, "REQ") == 0) {
            crowd_req(fifo, dept_id);
        } else if (token_count == 1 && strcmp(command, "PROCESS") == 0) {
            crowd_process(fifo, task, task_max_size);
        } else if (token_count == 2 && strcmp(command, "ASSIGNTIC") == 0) {
            crowd_assign(task, dept_id);
        } else if (token_count == 2 && strcmp(command, "FINDALL") == 0) {
            crowd_find(fifo, task, dept_id);
        } else if (token_count == 1 && strcmp(command, "PRINTREQ") == 0) {
            crowd_print_list(fifo, "FIFO");
        } else if (token_count == 1 && strcmp(command, "PRINTTIC") == 0) {
            crowd_print_list(task, "Task");
        } else if (token_count == 1 && strcmp(command, "STATS") == 0) {
            crowd_stats(fifo, task, task_max_size);
        } else if (token_count == 1 && strcmp(command, "QUIT") == 0) {
            crowd_cleanup(task);
            crowd_cleanup(fifo);
            //printf("Goodbye\n");
            break;
        } else if (token_count == 2 && strcmp(command, "SORT") == 0) {
            crowd_sort(&fifo, dept_id);
        } else {
            //printf("# %s", line);
            continue;
        }
    }
    exit(0);
}
/* commands specified to vim. ts: tabstop, sts: soft tabstop sw: shiftwidth */
/* vi:set ts=8 sts=4 sw=4 et: */
