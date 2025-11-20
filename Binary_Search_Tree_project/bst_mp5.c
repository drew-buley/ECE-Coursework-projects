/* Drew Buley
 * C20407096
 * MP5
 * 11/13/2024
 *
 * Purpose: This file contains functions for working with binary search trees
 *          The functions are called by driver lab5.c, and are supported
 *          by datatypes.h which contains important project definitions such
 *          as data structures and function prototypes
 *
 * Assumptions: As these functions are called by lab5, it is
 *              assumed that they are called with only vaild values
 *              (EX: tree pointer is only a tree created by construct())
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "datatypes.h"

/* constants used with Global variables */
#define MAX_HEIGHT 100
#define MAX_WIDTH 100
#define TRUE 1
#define FALSE 0

/* Purpose: creates a new bst header block
 * Inputs: None
 * Outputs: Pointer to new bst headerblock
*/
tree_t *bst_construct() {
  tree_t *new_tree = (tree_t *)malloc(sizeof(tree_t));
  new_tree->root = NULL;
  new_tree->tree_size = 0;
  new_tree->recent_comparisons = 0;
  return new_tree;
}

/* Purpose: Helper function for bst_destruct. Enables recursive
 *          traversal of the tree to free from bottom up
 * Inputs: tree node
 * Outputs: None
 *
 */
void destruct_helper(tree_node_t *t_node) {
    if (t_node->left != NULL) {
        destruct_helper(t_node->left);
    }
    if (t_node->right != NULL) {
        destruct_helper(t_node->right);
    }
    //node now has no children
    free(t_node->data_ptr);
    free(t_node);
}

/* Purpose: Free all memory in given list
 * Inputs: Pointer to bst header block
 * Outputs: None
*/
void bst_destruct(tree_t *tree) {
    assert(tree != NULL);
    tree_node_t *root = tree->root;
    if (root != NULL) {
        destruct_helper(root);
    }
    free(tree);
}

/* Purpose: Insert a new node into the tree or replace node already in tree
 * Inputs: tree header block, key to insert at, pointer to data to insert
 * Outputs: 0 if key was found and replace in tree, 1 if node was added to tree
 */

int bst_insert(tree_t *tree, tree_key_t key, mydata_t *data_ptr) {

    assert(tree != NULL);
    tree_node_t **current = &(tree->root);
    tree->recent_comparisons = 0;

    //loop until found correct node to insert at
    while (*current != NULL) {
        assert((*current)->key != 0);

        tree->recent_comparisons++;
        if ((*current)->key == key) {
            //update record
            mydata_t *old_data = (*current)->data_ptr;
            (*current)->data_ptr = data_ptr;
            free(old_data);
            //return 0 as found key in tree
            return 0;
        }

        tree->recent_comparisons++;
        //move pointer to current node's child
        if (key < (*current)->key) {
            current = &((*current)->left);
        } else if (key > (*current)->key) {
            current = &((*current)->right);
        }
    }
    
    // current is last pointer before null leaf
    *current = (tree_node_t *)malloc(sizeof(tree_node_t));

    (*current)->key = key;
    (*current)->data_ptr = data_ptr;
    (*current)->left = NULL;
    (*current)->right = NULL;

    tree->tree_size++;
    //return 1 as key added to tree
    return 1;
}

/* Purpose: Search tree for node with specified key.
 *          if it is found, remove it and when necessary promote a predecessor
 *          frees old node once data recovered and bst structure restored
 * Inputs: tree pointer, specified key
 * Outputs: pointer to the removed data or NULL if key was not found
 */

mydata_t *bst_remove(tree_t *tree, tree_key_t key) { 
    assert(tree != NULL);
    tree_node_t **current = &(tree->root);
    tree->recent_comparisons = 0;
    
    //search tree for key
    while ((*current != NULL) && ((*current)->key != key)) {
        tree->recent_comparisons++;
        if (key < (*current)->key) {
            current = &((*current)->left);
            tree->recent_comparisons++;
        } else {
            current = &((*current)->right);
            tree->recent_comparisons++;
        }
    }

    //key not found in tree
    if (*current == NULL) {
        return NULL;
    }

    mydata_t *data = (*current)->data_ptr;
    tree_node_t *old_node = *current;

    //remove node with no children
    if (((*current)->left == NULL) && ((*current)->right == NULL)) {
        *current = NULL;
    } 
    // Node with only left child
    else if ((*current)->right == NULL) {
        *current = (*current)->left;
    }
    // node with only right child
    else if ((*current)->left == NULL) {
        *current = (*current)->right;
    }
    //node that has both children
    else {
        assert(((*current)->left != NULL) && ((*current)->right != NULL));
        tree_node_t **predecessor = &(*current)->left;

        //loop until found largest node less than target
        while ((*predecessor)->right != NULL) {
            predecessor = &(*predecessor)->right;
            tree->recent_comparisons++;
        }
        (*current)->key = (*predecessor)->key;
        (*current)->data_ptr = (*predecessor)->data_ptr;

        old_node = *predecessor;
        *predecessor = (*predecessor)->left;
    }

    tree->tree_size--;
    free(old_node);
    return data;
}

/* Purpose: Searches tree for node with specified key
 * Input: tree pointer and specified key to search for
 * Output: pointer to the data block of specified node or NUll
 *         if not found.
 */

mydata_t *bst_search(tree_t *tree, tree_key_t key) {
    assert(tree != NULL);
    tree_node_t *rover = tree->root;
    tree->recent_comparisons = 0;

    while (rover != NULL) {
        tree->recent_comparisons++;
        if (rover->key == key) {
            return rover->data_ptr;
        }
        tree->recent_comparisons++;
        if (key < rover->key) {
            rover = rover->left;
        } else {
            rover = rover->right;
        }
    }
    return NULL;
}

/* Purpose: Determines the level of a specified key independent of
 *          if it actually is in the tree
 * Inputs: tree pointer, specified key to search for
 * OUtputs: level key is at or would be at if it was in the tree
 */

int bst_level(tree_t *tree, tree_key_t key) {
    assert(tree != NULL);

    int level = 0;
    tree_node_t *rover = tree->root;

    while (rover != NULL) {
        if (rover->key == key) {
            return level;
        }
        if (key < rover->key) {
            rover = rover->left;
            if (rover == NULL) { //found left null leaf
                return (level+1);
            }
        } else {
            rover = rover->right;
            if (rover == NULL) { //found right null leaf
                return (level + 1);
            }
        }
        level++;
    }
    return level; //should only run if root is NULL
}

/* Purpose: Determines the size of the tree
 * Inputs: tree pointer
 * OUtputs: size of the tree
 */
int bst_size(tree_t *tree) {
    return tree->tree_size;
}

/* Purpose: Determines the number of recent comparisons from the last
 *          insert, search, or remove
 * Inputs: tree pointer
 * Outputs: number of recent comparisons
 */
int bst_stats (tree_t *tree) {
    return tree->recent_comparisons;
}


/* Purpose: helper function for internal path length that enables
 *          recursive traversal
 * Inputs: node pointer, level pointer
 * Outputs: level of nodes below it
 */
int path_helper(tree_node_t *t_node, int level) {
    if (t_node == NULL) {
        return 0;
    } else {
        return (level + path_helper(t_node->left, level + 1) + path_helper(t_node->right, level + 1));
    }
}

/* Purpose: determines the internal path length of the tree
 * Inputs: tree pointer
 * Outputs: internal path length
 */
int bst_internal_path_len(tree_t *tree) {
    assert(tree != NULL);
    int len = path_helper(tree->root, 0);
    return (len);
}


// Credit for inital C++ code for pretty print which was adapted for this project:
// User: Lastchance (6980)
// site: https://cplusplus.com/forum/general/265712/#google_vignette
// Date initially posted: 11/27/2019
// Data adapted 11/13/2024
// Code description: Provided a 2D terminal representation of Binary trees
//------------------beginning of adapted code-----------------------
// helper function for pretty print, returns max of two ints
int max(int a, int b) {
    if (a > b) {
        return a;
    }
    else {
        return b;
    }
}

/* Purpose: determines the max distance from a null leaf for a provided node
 * Inputs: node pointer
 * Outputs: depth
 */
int depth(tree_node_t* t_node) {
    // returns max distance from NULL leaf
    if (t_node == NULL) 
        return 0;
    // recur on children
    return 1 + max(depth(t_node->left), depth(t_node->right));
}

// shift rows of return array by offset
void shift_rows(char rows[MAX_HEIGHT][MAX_WIDTH], int offset) {
    int i;
    for (i = 0; i < MAX_HEIGHT; i++) {
        if (strlen(rows[i]) > 0) {
            memmove(rows[i] + offset, rows[i], strlen(rows[i]) + 1);
            memset(rows[i], ' ', offset);
        }
    }
}

// calculate max position based on other rows
int get_max_position(char rows[MAX_HEIGHT][MAX_WIDTH], int level, int pos) {
    if (level < MAX_HEIGHT - 1) {
        if (strlen(rows[level + 1]) > pos) {
          pos = strlen(rows[level + 1]);
        }
    }
    if (level > 0) {
        if (strlen(rows[level - 1]) > pos) {
          pos = strlen(rows[level - 1]);
        }
    } 
    if (strlen(rows[level]) > pos) {
        pos = strlen(rows[level]);
    }
    return pos;
}

//add data to the output array at a specific place
void add_node_data(char output[MAX_HEIGHT][MAX_WIDTH], int level, int pos, int key) {
    char nodekeys[10];
    snprintf(nodekeys, sizeof(nodekeys), " %d ", key);
    int space = pos - strlen(output[level]);
    if (space > 0) {
        char spaces[space + 1];
        memset(spaces, ' ', space);
        spaces[space] = '\0';
        strcat(output[level], spaces);
    }
    strcat(output[level], nodekeys);
}
// add a link character above the current node key
void add_link_above(char link_above[MAX_HEIGHT][MAX_WIDTH], int level, int pos, char link_char) {
    int space = pos + 1 - strlen(link_above[level]);
    if (space > 0) {
        char spaces[space + 1];
        memset(spaces, ' ', space);
        spaces[space] = '\0';
        strcat(link_above[level], spaces);
    }
    strncat(link_above[level], &link_char, 1);
}

// recursively prints nodes
void print_tree_node(char output[MAX_HEIGHT][MAX_WIDTH], char link_above[MAX_HEIGHT][MAX_WIDTH], tree_node_t *t_node, int level, int pos, char link_char) {
    if (t_node == NULL)
        return;

    // shifting of rows
    if (pos < 0) 
        shift_rows(output, -pos);
    if (pos < 0) 
        shift_rows(link_above, -pos);

    // determine max position
    pos = get_max_position(output, level, pos);

    // print left child
    if (t_node->left) {
        int left_width = snprintf(NULL, 0, " %d ", t_node->left->key);
        print_tree_node(output, link_above, t_node->left, level + 1, pos - left_width, 'L');
        pos = get_max_position(output, level + 1, pos);
    }

    // Add the current node data to the output array
    add_node_data(output, level, pos, t_node->key);

    // Add the link character to linkAbove at this level
    add_link_above(link_above, level, pos, link_char);

    // Recursively draw the right child if it exists
    if (t_node->right)
        print_tree_node(output, link_above, t_node->right, level + 1, strlen(output[level]), 'R');
}

void print_full(tree_t* tree) {
    assert(tree != NULL);
    //get root node of tree
    tree_node_t *root = tree->root;

    // Get the height of the tree
    int d = depth(root);

    // Initialize the output and linkAbove arrays, each with h rows
    char output[MAX_HEIGHT][MAX_WIDTH] = { "" };
    char link_above[MAX_HEIGHT][MAX_WIDTH] = { "" };

    // Call drawNode to populate output and linkAbove with formatted tree structure
    print_tree_node(output, link_above, root, 0, 5, ' ');

    // Create connecting lines (links) between nodes
    int i, j, jj, k, size;
    for (i = 1; i < d; i++) {
        for (j = 0; j < strlen(link_above[i]); j++) {
            // Check for a non-space link character in linkAbove
            if (link_above[i][j] != ' ') {
                // Ensure output line above is extended to at least j+1 spaces if needed
                size = strlen(output[i - 1]);
                if (size < j + 1) {
                    memset(output[i - 1] + size, ' ', j + 1 - size);
                    output[i - 1][j + 1] = '\0';
                }

                // Variable for adjusting link connections
                jj = j;
                
                // Handle left link 'L' by connecting to the node's position above
                if (link_above[i][j] == 'L') {
                    // Move right to the nearest non-space character in output above
                    while (output[i - 1][jj] == ' ')
                        jj++;
                    // Fill the spaces in output with underscores ('_') from j+1 to jj-1
                    for (k = j + 1; k < jj - 1; k++)
                        output[i - 1][k] = '_';
                }
                // Handle right link 'R' by connecting to the node's position above
                else if (link_above[i][j] == 'R') {
                    // Move left to the nearest non-space character in output above
                    while (output[i - 1][jj] == ' ')
                        jj--;
                    // Fill the spaces in output with underscores ('_') from j-1 down to jj+1
                    for (k = j - 1; k > jj + 1; k--)
                        output[i - 1][k] = '_';
                }
                
                // After processing, replace link character with vertical connector '|'
                link_above[i][j] = '|';
            }
        }
    }

    // Print each level of the tree
    for (i = 0; i < d; i++) {
        // Print linkAbove row first if this is not the first level
        if (i > 0)
            printf("%s\n", link_above[i]);

        // Print the output row for the current level
        printf("%s\n", output[i]);
    }
}
//---------------------end of adapted code-----------------------

//ugly print and initial validate
/* commented out to test pretty print
void ugly_print(tree_node_t *N, int level) {
    if (N == NULL) return;
    ugly_print(N->right, level+1) ;
    for (int i=0; i<level; i++) printf("  ");// print 5 spaces
    printf("%5d\n", N->key); // field width is 5
    ugly_print(N->left, level+1);
}
*/
void bst_debug_print_tree(tree_t *T) {
    printf("Tree with %d keys\n", T->tree_size);
    //ugly_print(T->root, 0);
    print_full(T);
    printf("\n");
    //bst_debug_validate(T);
}

/* Purpose: helper function for validate
 */
int bst_debug_validate_rec(tree_node_t *N, int min, int max, int *count)
{
    if (N == NULL) return TRUE;
    if (N->key <= min || N->key >= max) return FALSE;
    assert(N->data_ptr != NULL);
    *count += 1;
    return bst_debug_validate_rec(N->left, min, N->key, count) &&
    bst_debug_validate_rec(N->right, N->key, max, count);
}
void bst_debug_validate(tree_t *T)
{
    int size = 0;
    assert(bst_debug_validate_rec(T->root, INT_MIN, INT_MAX, &size) == TRUE);
    assert(size == T->tree_size);
}
/* vi:set ts=8 sts=4 sw=4 et: */
