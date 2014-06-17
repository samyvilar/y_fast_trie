
#include <string.h>
#include "tree.h"
#include "bits.h"
#include <unistd.h>

#include "timed.h"

stack_array_def(tree_t *, 100, tree_stack);

tree_t *tree_block = NULL, *tree_recycled = NULL;
unsigned tree_block_cnt = 0;
tree_t *tree_alloc()
{
    if (is_not_null(tree_recycled)) {
        tree_t *tree = tree_recycled;
        tree_recycled = tree_right(tree_recycled);
        return tree;
    }

    if (tree_block_cnt)
        return &tree_block[--tree_block_cnt];

    tree_block_cnt = getpagesize() / sizeof(tree_t);
    tree_block = malloc(sizeof(tree_t) * tree_block_cnt);
    return &tree_block[--tree_block_cnt];
}
void tree_recyl(tree_t *tree)
{
    tree_set_right(tree, tree_recycled);
    tree_recycled = tree;
}

//tree_t trees[2143022 * 2], *trees_p = trees;
//#define tree_alloc() (trees_p++)
//#define tree_recyl(tree)



int tree_is_corrupt(tree_t *tree) // returns 1 if tree is corrupt eitherwise 0 if ok
{
    if (is_null(tree) || tree_is_empty(tree) || tree_is_leaf(tree))
        return 0;

    if ( // check keys ...
           (tree_key(tree_left(tree))  >= tree_key(tree))
        || (tree_key(tree_right(tree)) < tree_key(tree))
        || (tree_key(tree_left(tree))  >= tree_key(tree_right(tree)))
        )
    return 1;

    if (    // check heights ..
            (tree_height(tree) == tree_height(tree_left(tree)))
        ||  (tree_height(tree) == tree_height(tree_right(tree)))
        ||  ((tree_height(tree) != (tree_height(tree_left(tree)) + 1)) && (tree_height(tree) != (tree_height(tree_right(tree)) + 1)))
        ||  (((tree_height(tree_left(tree)) - tree_height(tree_right(tree))) != 0) && ((tree_height(tree_left(tree)) - tree_height(tree_right(tree))) != 1) && ((tree_height(tree_left(tree)) - tree_height(tree_right(tree))) != (tree_key_t)-1))
    )
        return 1;

    if (tree_height(tree) == 1 && tree_height(tree_left(tree)) != 0 && tree_height(tree_right(tree)) != 0)
        return 1;

    return tree_is_corrupt(tree_left(tree)) || tree_is_corrupt(tree_right(tree));
}


#define __tree_traversal(tree, func)

unsigned long tree_weight(tree_t *tree)
{
    if (__builtin_expect(is_null(tree), 0) || tree_is_empty(tree))
        return 0;
    if (tree_is_leaf(tree))
        return 1;

    unsigned long cnt = 0;
    stack_array_init(tree_stack);
    stack_array_push(tree_stack, tree);

    while (stack_array_is_not_empty(tree_stack)) {
        tree = stack_array_pop(tree_stack);
        while (tree_height(tree) / 2)
            switch (tree_height(tree) / 2) {
                default:
                case 15: stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree);
                case 14: stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree);
                case 13: stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree);
                case 12: stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree);
                case 11: stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree);
                case 10: stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree);
                case 9:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree);
                case 8:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree);
                case 7:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree);
                case 6:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree);
                case 5:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree);
                case 4:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree);
                case 3:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree);
                case 2:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree);
                case 1:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree);
            }
        if (tree_height(tree) == 2) {
            stack_array_push(tree_stack, tree_right(tree));
            tree = tree_left(tree);
        }
        else if (tree_height(tree) == 1)
            cnt += 2;
        else
            ++cnt;
    }

    return cnt;
}

unsigned long tree_size(tree_t *tree)
{
    if (__builtin_expect(is_null(tree), 0) || tree_is_empty(tree))
        return 0;
    if (tree_is_leaf(tree))
        return 1;

    unsigned long cnt = 0;
    stack_array_init(tree_stack);
    stack_array_push(tree_stack, tree);

    while (stack_array_is_not_empty(tree_stack)) {
        tree = stack_array_pop(tree_stack);
        while (tree_height(tree) / 2)
            switch (tree_height(tree) / 2) {
                default:
                case 15: stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree); cnt += 2;
                case 14: stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree); cnt += 2;
                case 13: stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree); cnt += 2;
                case 12: stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree); cnt += 2;
                case 11: stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree); cnt += 2;
                case 10: stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree); cnt += 2;
                case 9:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree); cnt += 2;
                case 8:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree); cnt += 2;
                case 7:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree); cnt += 2;
                case 6:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree); cnt += 2;
                case 5:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree); cnt += 2;
                case 4:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree); cnt += 2;
                case 3:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree); cnt += 2;
                case 2:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree); cnt += 2;
                case 1:  stack_array_push(tree_stack, tree_right(tree)); tree = tree_left(tree); cnt += 2;
            }
        if (tree_height(tree) == 2) {
            stack_array_push(tree_stack, tree_right(tree));
            tree = tree_left(tree);
            cnt += 2;
        }
        else if (tree_height(tree) == 1)
            cnt += 2;
        else
            ++cnt;
    }

    return cnt;
}

void tree_insert(tree_t *tree, tree_key_t key, void *value)
{
    if (__builtin_expect(is_null(tree), 0))
        return ;

    if (tree_is_empty(tree)) {
        tree_leaf_init(tree, key, value);
        return ;
    }

    tree_find_path_macro(tree, key);

    if (tree_key(tree) == key)  {
        tree_set_value(tree, value);
        return ;  // key found, update value ... no re-balancing required.
    }

    if (key >= tree_key(tree)) { // check which side the new key goes if it goes on the right swap key, value and continue.
        swap(value, tree_left(tree));
        swap(key, tree_key(tree));
    }
    // the following assumes that the new key, value pair goes on the left ...
    *tree_set_right(tree, tree_alloc()) = (tree_t){._right = NULL, ._left = tree_value(tree), ._key = tree_key(tree), ._height = 0};
    *tree_set_left(tree, tree_alloc()) = (tree_t){._right = NULL, ._left = value, ._key = key, ._height = 0};
    tree_inc_height(tree);

    tree_balnc_macro(tree);
}

// returns value if present NULL if not present ...
void *tree_del(tree_t *tree, tree_key_t key)
{
    if (__builtin_expect(is_null(tree) || tree_is_empty(tree), 0))
        return NULL;

    tree_find_path_macro(tree, key);

    if (tree_key(tree) != key)
        return NULL;

    void *value = tree_value(tree);
    if (stack_array_is_empty(tree_stack)) {
        tree_mark_empty(tree);
        return value;
    }

    tree_t *other_child = (void *)(
        cast_ptr(tree_left(stack_array_peek(tree_stack)))
     ^ cast_ptr(tree_right(stack_array_peek(tree_stack)))
     ^ cast_ptr(tree)
    );  // get the other pointer by using xor operator since A ^ A = 0 and B ^ 0 = B

    *stack_array_pop(tree_stack) = *other_child; // copy the contents of the other child to parent ...

    tree_recyl(tree); tree_recyl(other_child); // recycle both nodes ...

    tree_balnc_macro(tree);

    return value;
}

// returns value if present otherwise NULL ...
void *tree_query(tree_t *tree, tree_key_t key) // returns value or NULL if not present ...
{
    if (__builtin_expect(is_null(tree) || tree_is_empty(tree), 0))
        return NULL;

    tree_find_leaf_macro(tree, key);

    return (tree_key(tree) == key) ? tree_value(tree) : NULL;
}


int tree_not_eq(tree_t *tree_a, tree_t *tree_b)
{
    if ((tree_key(tree_a) != tree_key(tree_b)) || (tree_height(tree_a) != tree_height(tree_b)))
        return 1;
    if (tree_is_leaf(tree_a) && tree_is_not_leaf(tree_b))
        return 1;
    if (tree_is_leaf(tree_b) && tree_is_not_leaf(tree_a))
        return 1;
    if (tree_is_leaf(tree_a) && tree_is_leaf(tree_b))
        return tree_value(tree_a) != tree_value(tree_b);

    return tree_not_eq(tree_left(tree_a), tree_left(tree_b)) || tree_not_eq(tree_right(tree_a), tree_right(tree_b));
}


void test_tree()
{
    // test NULL inputs.
    tree_insert(NULL, 1232, (void *)123213);
    if (tree_del(NULL, 1242))
        printf("bad delete\n"), exit(-1);
    if (tree_query(NULL, 2422))
        printf("bad query\n"), exit(-1);
    if (tree_weight(NULL))
        printf("tree bad cnt\n"), exit(-1);

    // test INSERTS/QUERIES/DELETE
    #define TREE_TEST_SIZE 1000000
    tree_key_t *keys = malloc(sizeof(tree_key_t) * TREE_TEST_SIZE);
    void **values = malloc(sizeof(void *) * TREE_TEST_SIZE);
    unsigned *rand_entries = malloc(sizeof(unsigned) * TREE_TEST_SIZE);

    tree_t *bad_tree = &(tree_t){NULL}, *empty_tree = &(tree_t){NULL};
    // test empty_tree queries/deletes
    if (tree_weight(bad_tree))
        printf("tree bad cnt\n"), exit(-1);
    if (tree_query(bad_tree, 1232))
        printf("bad query\n"), exit(-1);
    if (memcmp(bad_tree, empty_tree, sizeof(*bad_tree))) // check that empty query hasn't updated the initial tree ..
        printf("bad query\n"), exit(-1);
    if (tree_del(bad_tree, 12242))
        printf("bad delete\n"), exit(-1);
    if (memcmp(bad_tree, empty_tree, sizeof(*bad_tree))) // check that empty query hasn't updated the initial tree ..
        printf("bad delete\n"), exit(-1);
    // test single insert/query/delete
    tree_insert(bad_tree, 1243, (void *) 1243);
    if (tree_weight(bad_tree) != 1)
        printf("bad tree cnt\n"), exit(-1);
    if (tree_is_not_leaf(bad_tree) || tree_is_empty(bad_tree))
        printf("bad insert\n"), exit(-1);
    if (tree_query(bad_tree, 521312) || (tree_query(bad_tree, 1243) != (void *)1243) || tree_query(bad_tree, 52131243))
        printf("bad query\n"), exit(-1);
    if (tree_del(bad_tree, 3431243) || tree_is_empty(bad_tree) || (tree_del(bad_tree, 1243) != (void *)1243) || tree_is_not_empty(bad_tree))
        printf("bad delete\n"), exit(-1);
    if (tree_weight(bad_tree))
        printf("bad tree cnt\n"), exit(-1);

    unsigned index;
    for (index = 0; index < TREE_TEST_SIZE; index++)
    {
        keys[index] = rand();
        values[index] = (void *) cast_ptr(rand());
        rand_entries[index] = rand() % TREE_TEST_SIZE;
    }

    tree_t *tree = &(tree_t){NULL};

    #define test_tree_insert() ({for (index = 0; index < TREE_TEST_SIZE; index++) tree_insert(tree, keys[index], values[index]);})
    double insert_time = timed(test_tree_insert);
    unsigned long tree_node_cnt = tree_size(tree);
    if (tree_is_corrupt(tree))
        printf("bad tree\n"), exit(-1);

    unsigned long cnt;
    #define test_tree_weight() (cnt = tree_weight(tree))
    double cnt_time = timed(test_tree_weight);

    #define test_tree_query() ({\
        for (index = 0; index < TREE_TEST_SIZE; index++)\
            if (__builtin_expect(tree_query(tree, keys[rand_entries[index]]) != values[rand_entries[index]], 1))\
                printf("tree query test failed!\n"), exit(-1);})
    double query_time = timed(test_tree_query);

    #define test_tree_del() ({for (index = 0; index < TREE_TEST_SIZE; index++) tree_del(tree, keys[index]);})
    double del_time = timed(test_tree_del);

    printf(
        "\ntree_t test_size: %i(key/value cnt) %lu(bytes) \n"
        "tree_size: %lu(trees), %lu(bytes) tree_weight: %lu(nodes), efficiency: %.4f\n"
        "tree_cnt_time: %.4f, insert_time: %.4f query_time %.4f delete_time: %.4f\n",
        TREE_TEST_SIZE,
        (TREE_TEST_SIZE * (sizeof(void *) + sizeof(tree_key_t))),
        tree_node_cnt,
        (tree_node_cnt * sizeof(*tree)), cnt,
        (TREE_TEST_SIZE * (sizeof(void *) + sizeof(tree_key_t)))/(double)(tree_node_cnt * sizeof(*tree)),
        cnt_time,
        insert_time, query_time, del_time);

    free(keys);
    free(values);
    free(rand_entries);
}

