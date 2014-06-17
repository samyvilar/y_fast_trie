

#if !defined (__TREE__)
#define __TREE__

#include <stdlib.h>
#include <stdio.h>
#include "stack.h"
#include "mem.h"
#include "bits.h"

#define tree_key_t int
#define tree_height_t char // a balanced tree w/ height >= 256, would mean it contains at least 2**256 nodes, really not possible, yet ...

//#define tree_max_height 37 // a height balanced tree with a height of 36 would require 2**36 nodes and contain roughly 39,088,169 leaves, with each node being 24 bytes would require 1TB

typedef struct tree_t { // Insertions/Query/Deletion/Membership all take O(1.44 * log n) space is
    struct tree_t *_right, *_left;
    tree_key_t      _key;
    tree_height_t   _height;
} tree_t;
#define tree_left(tree)             ((tree)->_left)
#define tree_right(tree)            ((tree)->_right)

#define tree_cmp_keys                   less_than
//#define tree_select(tree, key)      ((typeof(*tree) **)(tree))[key >= tree_key(tree)] //(((typeof(*tree) **)(tree))[tree_cmp_keys(key, tree_key(tree))]) // branchless, assumes specific member order!
//#define tree_select(tree, key)      ((typeof(tree) *)(tree))[(tree_key(tree) - key) >> (bit_size(key) - 1)] //(((typeof(*tree) **)(tree))[tree_cmp_keys(key, tree_key(tree))]) // branchless, assumes specific member order!
//#define tree_select(tree, key)      (((typeof(*tree) **)(tree))[key >= tree_key(tree)])
//#define tree_select(tree, key) (*(((typeof(*tree) **)(tree)) + (((key - tree_key(tree)) >> (bit_size(key) - 1))))) // branchless, assumes specific member order!
//#define tree_select(tree, key) (((typeof(*tree) **)(tree))[key < tree_key(tree)]) // branchless, assumes specific member order!
#define tree_select(tree, key) (((key) < tree_key(tree)) ? tree_left(tree) : tree_right(tree))

#define tree_set_left(tree, l)      (tree_left(tree) = (l))
#define tree_set_right(tree, r)     (tree_right(tree) = (r))
#define tree_key(tree)              ((tree)->_key)
#define tree_set_key(tree, k)       (tree_key(tree) = (k))
#define tree_height(tree)           ((tree)->_height)
#define tree_inc_height(tree)       (++tree_height(tree))
#define tree_set_height(tree, h)    (tree_height(tree) = (h))
#define tree_value(tree)            ((void *)tree_left(tree))
#define tree_set_value(tree, v)     tree_set_left(tree, (void *)(v))
#define tree_leaf_init(tree, key, value) (*(tree) = (tree_t){._key = (key), ._left = (void *)(value), ._right = NULL, ._height = 0}) //({tree_set_right(tree, NULL); tree_set_value(tree, value); tree_set_key(tree, key); tree_set_height(tree, 0);})

#define tree_mark_empty(tree)       tree_set_left(tree, NULL)
#define tree_is_empty(tree)         is_null(tree_left(tree))
#define tree_is_not_empty(tree)     is_not_null(tree_left(tree))

#define tree_mark_leaf(tree)        tree_set_right(tree, NULL)
#define tree_is_leaf(tree)          is_null(tree_right(tree))
#define tree_is_not_leaf(tree)      is_not_null(tree_right(tree))

#define tree_upd_height(tree) tree_set_height(tree, max(tree_height(tree_left(tree)), tree_height(tree_right(tree))) + 1)

#define tree_height_diff(tree_a, tree_b)   ((tree_height(tree_a) - tree_height(tree_b)))

#define tree_left_rot(tree) {\
    typeof(tree) right_tree = tree_right(tree);\
    swap(tree_key(tree),         tree_key(right_tree));\
    swap(tree_left(tree),        tree_right(tree));\
    swap(tree_right(right_tree), tree_left(right_tree));\
    swap(tree_right(tree),       tree_left(tree_left(tree)));\
}

//#define tree_left_rot(tree) {\
//    typeof(tree) orig_left_tree = tree_left(tree), orig_right_tree = tree_right(tree);\
//    typeof(tree_key(tree)) root_key = tree_key(tree);\
//    tree_set_left(tree, orig_right_tree);\
//    tree_set_right(tree, tree_right(orig_right_tree));\
//    tree_set_right(orig_right_tree, tree_left(orig_right_tree));\
//    tree_set_left(orig_right_tree, orig_left_tree);\
//    tree_set_key(tree, tree_key(orig_right_tree));\
//    tree_set_key(orig_right_tree, root_key);\
//}

//({asm("xchgl %0, %1" : "=r"(a), "=r"(b) : "0"(a), "1"(b));})
//{
//    swap_4_xchng(tree_key(tree), tree_key(right_tree)); \
//    asm (
//        "xchgq %0, %1;\t\nxchgq %2, %3;\t\nchgq %4, %5;\t\n"
//        : "=r"(a), "=r"(b), "=r"(c), "=r"(d), "=r"(e), "=r"(f)
//        : "0"(a), "1"(b), "2"(c), "3"(d), "4"(e), "5"(f)
//    );
//}


#define tree_right_rot(tree) { \
    typeof(tree) left_tree = tree_left(tree);                               \
    swap(tree_key(tree),       tree_key(left_tree));                        \
    swap(tree_left(tree),      tree_right(tree));                           \
    swap(tree_left(left_tree), tree_right(left_tree));                      \
    swap(tree_left(tree),      tree_right(tree_right(tree)));               \
}

//#define tree_right_rot(tree) { \
//    typeof(tree) orig_left_tree = tree_left(tree), orig_right_tree = tree_right(tree);  \
//    typeof(tree_key(tree)) root_key = tree_key(tree);\
//    tree_set_right(tree, orig_left_tree);\
//    tree_set_left(tree, tree_left(orig_left_tree));\
//    tree_set_left(orig_left_tree, tree_right(orig_left_tree));\
//    tree_set_right(orig_left_tree, orig_right_tree);\
//    tree_set_key(tree, tree_key(orig_left_tree));\
//    tree_set_key(orig_left_tree, root_key);\
//}

//#undef tree_right_rot
//#define tree_right_rot right_rotation

#define tree_right_upd_heights(tree) (\
    tree_set_height(tree_right(tree), tree_height(tree_right(tree_right(tree))) + 1),\
    tree_set_height(tree, tree_height(tree_right(tree)) + 1)\
)

//tree_set_height(tree_right(tree), max(tree_height(tree_right(tree_right(tree))), tree_height(tree_left(tree_right(tree)))) + 1),\
    //tree_set_height(tree, max(tree_height(tree_right(tree)), tree_height(tree_left(tree)) + 1))\
//)
#define tree_right_rot_wth_heights(tree) (tree_right_rot(tree), tree_right_upd_heights(tree))

//    tree_set_height(tree_left(tree), max(tree_height(tree_right(tree_left(tree))), tree_height(tree_left(tree_left(tree)))) + 1),\
//    tree_set_height(tree, max(tree_height(tree_right(tree)), tree_height(tree_left(tree))) + 1)\
//)
#define tree_left_rot_wth_heights(tree) (tree_left_rot(tree), tree_left_upd_heights(tree))

#define TREE_LEFT_LEFT_HEAVY         2
#define TREE_LEFT_HEAVY              1
#define TREE_BALANCED                0
#define TREE_RIGHT_HEAVY            -1
#define TREE_RIGHT_RIGHT_HEAVY      -2

#define tree_right_left_heavy(tree)

#define tree_balnc_macro(tree) {\
while (stack_array_is_not_empty(tree_stack))  {\
    tree = stack_array_pop(tree_stack);\
    tree_height_t prev_height = tree_height(tree), height_delta = tree_height_diff(tree_left(tree), tree_right(tree));\
    \
    if (height_delta == TREE_RIGHT_RIGHT_HEAVY) {\
        if (tree_height(tree_left(tree)) == tree_height(tree_right(tree_right(tree)))) {/*right-left heavy ...*/\
            tree_right_rot(tree_right(tree));\
            tree_left_rot(tree);\
            \
            tree_set_height(tree_left(tree), tree_height(tree_right(tree_right(tree))) + 1);\
            tree_set_height(tree_right(tree), tree_height(tree_right(tree_right(tree))) + 1);\
            tree_set_height(tree, tree_height(tree_right(tree_right(tree))) + 2);\
        }\
        else {\
            tree_left_rot(tree);\
        \
            tree_set_height(tree_left(tree), tree_height(tree_right(tree_left(tree))) + 1);\
            tree_set_height(tree, tree_height(tree_right(tree_left(tree))) + 2);\
        }\
    }\
    else if (height_delta == TREE_LEFT_LEFT_HEAVY) {\
        if (tree_height(tree_right(tree)) == tree_height(tree_left(tree_left(tree)))) {\
            tree_left_rot(tree_left(tree));\
            tree_right_rot(tree);\
            \
            tree_set_height(tree_left(tree), tree_height(tree_left(tree_left(tree))) + 1);\
            tree_set_height(tree_right(tree), tree_height(tree_left(tree_left(tree))) + 1);\
            tree_set_height(tree, tree_height(tree_left(tree_left(tree))) + 2);\
        }\
        else {\
            tree_right_rot(tree);\
            \
            tree_set_height(tree_right(tree), tree_height(tree_left(tree_right(tree))) + 1);\
            tree_set_height(tree, tree_height(tree_left(tree_right(tree))) + 2);\
        }\
    }\
    else if (height_delta == TREE_LEFT_HEAVY)\
        tree_set_height(tree, tree_height(tree_left(tree)) + 1);\
    else\
        tree_set_height(tree, tree_height(tree_right(tree)) + 1);\
    \
    if (prev_height == tree_height(tree)) break ;\
}   }



// since we are using a leaf tree, ie all the value are at the leafs, we can use the height to determine how many selects
// we can apply sequentially everytime we select a node we can shrink our path up to 2 nodes, since maximum diff between child
// nodes cannot differ by more than 2, and the parents height is one greater than the max of its children ...
#define __tree_find_leaf(tree, key, func)\
    while (tree_height(tree) / 2)\
        switch (tree_height(tree) / 2) {\
            default:\
            case 18: func(tree); tree = tree_select(tree, key);\
            case 17: func(tree); tree = tree_select(tree, key);\
            case 16: func(tree); tree = tree_select(tree, key);\
            case 15: func(tree); tree = tree_select(tree, key);\
            case 14: func(tree); tree = tree_select(tree, key);\
            case 13: func(tree); tree = tree_select(tree, key);\
            case 12: func(tree); tree = tree_select(tree, key);\
            case 11: func(tree); tree = tree_select(tree, key);\
            case 10: func(tree); tree = tree_select(tree, key);\
            case 9:  func(tree); tree = tree_select(tree, key);\
            case 8:  func(tree); tree = tree_select(tree, key);\
            case 7:  func(tree); tree = tree_select(tree, key);\
            case 6:  func(tree); tree = tree_select(tree, key);\
            case 5:  func(tree); tree = tree_select(tree, key);\
            case 4:  func(tree); tree = tree_select(tree, key);\
            case 3:  func(tree); tree = tree_select(tree, key);\
            case 2:  func(tree); tree = tree_select(tree, key);\
            case 1:  func(tree); tree = tree_select(tree, key);\
        }\
    if (tree_height(tree)) {func(tree); tree = tree_select(tree, key);}

#define consume(x)
#define tree_find_leaf_macro(tree, key) __tree_find_leaf(tree, key, consume)

stack_array_def(extern tree_t *, 100, tree_stack);
#define _tree_trace_path(tree) stack_array_push(tree_stack, tree)

#define tree_find_path_macro(tree, key) \
    stack_array_init(tree_stack); \
    __tree_find_leaf(tree, key, _tree_trace_path)


// faster tree but consumes more space keys are broken up to byte sequence ..
// giving a key comprise of 32 bits
#define FAST_TREE_KEY_BIT_SIZE 5
// combines a bitfield/alphabet compression/height balance tree to create faster tree structure ...
// normally a 32 bit key would require 2**32 - 1 bits or 540MB bitfield not very practical ...
// if our key(s) where smaller lets say only 5 or 6 bits, we would need only 4 or 8 bytes respectively ...
#define fast_tree_height_t tree_height_t
#define fast_tree_key_t char


typedef struct fast_tree_t { // 5 bit trie ...
    struct fast_tree_t ** _nodes; // if NULL tree is empty, otherwise its either a full tree or a leaf ...
    unsigned _field;
    char _key, _height; // if _height == 0 then its a leaf and the value is at __nodes and key is at _field, otherwise its a full tree ..
} fast_tree_t;
// a full tree contains at least two leafs: contained somewhere down below
// _field is a bitfield that contains 2**5 entries, so we can check in contant time whether or not a previous key was inserted
// we select _nodes by taking the index of the most significant bit of that 5 bit index and continue our decent until
// we hit an empty tree
// _nodes is non-null and contains 5 pointers ...

// to query while current tree is not a leaf, check which of the 5 nodes to select by taking the key

#define fast_tree_nodes(ft) ((ft)->_nodes)
#define fast_tree_set_nodes(ft, n) (fast_tree_nodes(ft) = (n))

#define fast_tree_height(ft) ((ft)->_height)
#define fast_tree_set_height(ft, height) (fast_tree_height(ft) = (height))
#define fast_tree_is_not_leaf(ft) (fast_tree_height(ft))
#define fast_tree_is_leaf !fast_tree_is_not_leaf

#define fast_tree_value(ft) ((void *)fast_tree_nodes(ft))
#define fast_tree_set_value(ft, v) (fast_tree_value(ft) = (v))

#define fast_tree_field(ft) ((ft)->_field)
#define fast_tree_set_field(ft, fld) (fast_tree_field(ft) = (fld))
#define fast_tree_leaf_key fast_tree_field
#define fast_tree_leaf_set_key fast_tree_set_field

#define fast_tree_is_empty(ft) (!fast_tree_field(ft))
#define fast_tree_init_leaf(ft, key, value) ({\
    fast_tree_leaf_set_key(ft, key); \
    fast_tree_set_value(ft, value); \
    fast_tree_set_height(ft, 0);\
})


void tree_insert(tree_t *, tree_key_t key, void *value); //, tree_t *(*tree_alloc)());
void *tree_del(tree_t *tree, tree_key_t key);
void *tree_query(tree_t *tree, tree_key_t key); // returns value or NULL if not present ...
unsigned long tree_weight(tree_t *tree);
unsigned long tree_size(tree_t *tree);
void test_tree();


#endif
