/*
 * Drew Buley
 * MP5
 * C20407096
 * 11/13/2024
 *
 * Purpose: Contains data structure and function prototypes
 * for MP5
 *
 * Assumptions: Changes to this file will be reflected
 *              in files that call these structures or functions.
 *              Additionally, functions called using tree* will 
 *              send a valid tree created by bst_construct()
*/

// -------- data storage container ----- //
typedef int mydata_t;


// ------ Data structure specific ------ //
typedef int tree_key_t;

//BST node
typedef struct NodeTag {
  tree_key_t key;
  mydata_t *data_ptr;
  struct NodeTag *left;
  struct NodeTag *right;
} tree_node_t;

//tree header
typedef struct TreeTag {
  tree_node_t *root;
  int tree_size;
  int recent_comparisons;
} tree_t;


// ---------- function prototypes --------- //
//tree management
tree_t *bst_construct();
void bst_destruct(tree_t *tree);

//node management
int bst_insert(tree_t *tree, tree_key_t key, mydata_t *data_ptr);
mydata_t *bst_remove(tree_t *tree, tree_key_t key);

//tree searching
mydata_t *bst_search(tree_t *tree, tree_key_t key);
int bst_level(tree_t *tree, tree_key_t key);

//tree stats
int bst_size(tree_t *tree);
int bst_stats (tree_t *tree);
int bst_internal_path_len(tree_t *tree);

//tree debugging
void bst_debug_print_tree(tree_t * tree);
void bst_debug_validate(tree_t *tree);

