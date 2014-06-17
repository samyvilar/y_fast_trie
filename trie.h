

#if !defined(__TRIE__)
#define __TRIE__

#include <limits.h>

#include "bits.h"
#include "tree.h"
#include "seq.h"
#include "mem.h"
#include "hash.h"

typedef struct x_fast_trie_t {
     hash_tbl_t *_levels[32];
} x_fast_trie_t;

#define trie_seq_bit_size 8

typedef struct trie_t {
    void *value;
    packed_bits_t seq;
    bit_fld_def(node_ids, pow_2(trie_seq_bit_size) + 1); // add an extra bit for -1 entry signaling end_of_seq
    tree_t edges; // tree to actually look up the edges ...
} trie_t;


#define trie_end_markr_symbl (((unsigned int)((trie_symbl_t)-1)) + 1)

#define trie_get_end_markr() (&trie_end_markr)

#define trie_is_null                        is_null
#define trie_is_good                        is_not_null

#define trie_seq_strt_p(node)               ((node)->seq_strt_p)
#define trie_seq_end_p(node)                ((node)->seq_end_p)
#define trie_seq_cnt(node)                  (trie_seq_strt_p(node) - trie_seq_end_p(end))

#define trie_set_seq_strt_p(node, s)        (trie_seq_strt_p(node) = (s))
#define trie_set_seq_end_p(node, e)         (trie_seq_end_p(node) = (e))
#define trie_set_seq(node, s, e)            (trie_set_seq_strt_p(node, s), trie_set_seq_end_p(node, e))
#define trie_seq_frst_symb(node)            (*trie_seq_strt_p(node))
#define trie_seq_len(node)                  (trie_seq_end_p(node) - trie_seq_strt_p(node))

#define trie_edges(node)                    (&((node)->edges))
#define trie_node_ids(node)                 ((node)->node_ids)

#define trie_is_edge_set(node, edge_id)     bit_fld_bit(trie_node_ids(node), edge_id)
#define trie_add_edge_id(node, edge_id)     bit_fld_set_bit(trie_node_ids(node), edge_id)
#define trie_rem_edge_id(node, edge_id)     bit_fld_clr_bit(trie_node_ids(node), edge_id)

#define trie_is_edge_set_safe(node, edge_id)        (trie_is_good(node) ? trie_is_edge_set(node, edge_id) : 0)
#define trie_is_edge_not_set(node, edge_id)         (!trie_is_edge_set(node, edge_id))
#define trie_is_edge_not_set_safe(node, edge_id)    (trie_is_good(node) ? trie_is_edge_not_set(node, edge_id) : 1)

#define trie_add_edge(node, symbl, edge, tree_alloc) (\
    trie_add_edge_id(node, symbl),\
    (tree_is_empty(trie_edges(node)) \
        ? tree_leaf_init(trie_edges(node), symbl, edge) \
        : tree_non_empty_insrt(trie_edges(node), symbl, edge, tree_alloc(), tree_alloc()))\
)
#define trie_mark_as_leaf(node, tree_alloc)         trie_add_edge(node, trie_end_markr_symbl, trie_get_end_markr(), tree_alloc)
#define trie_mark_as_non_leaf_safe(node, tree_free) trie_rem_edge_safe(trie, trie_end_markr_symbl, free_tree)

#define trie_rem_edge(node, symbl, tree_recl) (\
    trie_rem_edge_id(node, symbl), \
    (tree_is_leaf(trie_edges(node)) \
        ? tree_set_value(trie_edges(node), NULL) \
        : tree_non_empty_del(trie_edges(node), symbl, tree_recl))\
)

#define trie_rem_edge_safe(node, symbl, tree_recl)(\
    (trie_is_edge_set_safe(node, symbl) && trie_rem_edge(node, symbl, tree_recl)) \
)

#define trie_upd_edge(node, symbl, edge) tree_upd(trie_edges(node), symbl, edge)

#define trie_edge(trie, symbl)      tree_query_safe(trie_edges(trie), symbl)
#define trie_edge_safe(trie, symbl) (trie_is_good(trie) ? trie_edge(trie, symbl) : NULL)
#define trie_has_no_edge(node)      (tree_is_empty(trie_edges(node)))
#define trie_has_single_edge(node)  (tree_is_leaf(trie_edges(node)))
#define trie_get_only_edge(node)    (tree_value(trie_edges(node)))

#define trie_is_leaf(node)  (\
    tree_is_empty(trie_edges(node)) || (trie_edge_safe(trie, trie_end_markr_symbl) == trie_get_end_markr())\
)
#define trie_is_leaf_safe(trie)     (trie_is_good(trie) && trie_is_leaf(trie))
#define trie_is_not_leaf(trie)      (!trie_is_leaf(trie))

#define trie_find_leaf(trie, suffix, seq) \
while (seq_cnt && trie_is_edge_set(trie, *seq))\
{\
    trie = trie_edge(trie, *seq);\
    suffix = trie_seq_strt_p(trie);\
    while (seq_cnt && (suffix < trie_seq_end_p(trie)) && (*suffix == *seq)) --seq_cnt, ++seq, ++suffix;\
}



#endif