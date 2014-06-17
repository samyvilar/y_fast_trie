// possible simple trie:
// struct trie_t {struct simple_trie_t *next[256];}
// simple and fast O(1) look up time, but requires initializing 256 pointers and on 64 bit arch its 2048 bytes.
// struct simple_trie_t {unsigned char ch; struct simple_trie_t *next, *current;}
//  more complex, O(k) both look up and check but on 64 bit arch only requires 12 bytes
// struct simple_trie_t {unsigned char ch; bit_fld_t set_nodes[mag]; unsigned struct simple_trie_t *next, *current;}
//  slightly more complex but at least we can check in constant time whether or not there is a node to the next field.
// required space jumps 32 bytes total of 44 bytes

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "trie.h"
//
//
//int trie_quere(trie_t *trie, seq_t *seq) // return non-zero if present, zero if not-present ...
//{
//
//    if (trie_is_good(trie) && seq_is_good(seq))
//        trie_find_leaf(trie, suffix, seq);
//
//    return !seq_cnt && trie_is_leaf_safe(trie) && (suffix == trie_seq_end_p(trie)); // if exhausted all elems check if we are at a leaf ...
//}
//


//void trie_insrt(trie_t *trie, packed_bits_t *seq, unsigned elem_bit_mag) {
//    if (__builtin_expect(is_null(trie) || is_null(seq)  || !elem_bit_mag || packed_bits_empty(seq) || elem_bit_mag > packed_bits_bit_cnt(seq), 0)
//        return ;
//
//    unsigned long index;
//    for (index = 0; index < cnt; index++) {
//        tree_key_t edge_id;
//    }
//    while (cnt && trie_is_edge_set(trie, *seq))
//    {
//        trie = trie_edge(trie, *seq);
//        edge_suffix = trie_seq_strt_p(trie);
//
//        unsigned long eq_symbl_cnt = seq_eq_byte_cnt(
//            &(seq_t){.start_p = seq, .end_p = seq + seq_cnt},
//            &(seq_t){.start_p = edge_suffix, .end_p = trie_seq_end_p(trie)}
//        ) / sizeof(trie_symbl_t);
//
//        seq_cnt -= eq_symbl_cnt;
//        edge_suffix += eq_symbl_cnt;
//        seq += eq_symbl_cnt;
//
//    }
//
//    if (seq_cnt)  // add any reminaing values ...
//    {
//        trie_t *edge = init_trie(alloc_trie(), seq, seq + seq_cnt);
//        trie_mark_as_leaf(edge, alloc_tree);
//        trie_add_edge(trie, *seq, edge, alloc_tree); // add edge
//
//        if (edge_suffix < trie_seq_end_p(trie)) // we have a remianing suffix, split node ...
//        {
//            edge = init_trie(alloc_trie(), edge_suffix, trie_seq_end_p(trie));
//            trie_mark_as_leaf(edge, alloc_tree);
//
//            trie_add_edge(trie, *edge_suffix, edge, alloc_tree);
//            trie_set_seq_end_p(trie, edge_suffix);
//
//            trie_mark_as_non_leaf_safe(trie, free_tree);  // remove end of seq marker if present ...
//        }
//    }
//    else if (trie_is_not_leaf(trie)) // no more remaining values check if end marker is present if not add it ..
//        trie_mark_as_leaf(trie, alloc_tree);
//    else    // no remaining values seq must already be present so return NULL.
//        return NULL;
//
//    return seq;
//}

//trie_symbl_t *trie_del( // return seq if successfull eitherwise NULL ...
//    trie_t *trie,
//    trie_symbl_t *seq,
//    unsigned long seq_cnt,
//    void (*free_tree)(),
//    void (*free_trie)()
//)
//{
//    if (seq_is_empty(seq, seq_cnt))
//        return seq;
//    if (trie_is_bad(trie) || seq_is_bad(seq, seq_cnt))
//        return NULL;
//
//
//    unsigned long index = 0;
//    trie_symbl_t *suffix = NULL;
//    trie_t *parent;
//    while ((index < seq_cnt) && trie_is_edge_set(trie, seq[index]))
//    {
//        parent = trie;
//        trie = trie_edge(parent, seq[index]);
//        suffix = trie_seq_strt_p(trie);
//        while ((index < seq_cnt) && (suffix < trie_seq_end_p(trie)) && (seq[index] == *suffix))
//            ++index, ++suffix;
//    }
//
//    if ((index != seq_cnt) && (suffix != trie_seq_end_p(trie)) && trie_is_not_leaf(trie))
//        return NULL; // seq not found ...
//
//    trie_rem_edge(trie, trie_end_markr_symbl, free_tree);
//
//    if (trie_has_no_edge(trie)) // if by removing end of sequence symb we have removed all edges remove this trie
//        trie_rem_edge_id(parent, trie_seq_frst_symb(trie)); // remove edge from parent ...
//    else if (trie_has_single_edge(trie)) // trie has a single child so we have to merge parent with grand child ...
//    {
//        trie_t *child = trie_get_only_edge(trie);
//        trie_set_seq_strt_p(child, trie_seq_strt_p(child) - trie_seq_len(trie));
//        if (trie_seq_frst_symb(child) != trie_seq_frst_symb(trie))
//        {   printf("Unable to merge tries!\n"); exit(-1);   }
//        trie_upd_edge(parent, trie_seq_frst_symb(child), child);
//    }
//    free_trie(trie);
//
//    return seq;
//}
//
//struct trie_t *trie_init(trie_t *trie, trie_symbl_t *start, trie_symbl_t *end)
//{
//    zero_out(trie, sizeof(*trie));
//    trie_set_seq(trie, start, end);
//    return trie;
//}
//
//trie_t *trie_alloc()   { return malloc(sizeof(trie_t)); }
//void trie_recyl(trie_t *trie) { free(trie); }
//
//void test_trie()
//{
//    unsigned long test_size = 10000, max_seq = 1000, index;
//    trie_symbl_t *symbols = malloc(test_size * max_seq);
//    for (index = 0; index < (test_size * max_seq); index++)
//        symbols[index] = rand() % (1 << BYTE_BIT_SIZE * sizeof(trie_symbl_t));
//    trie_t trie;
//    trie_init(&trie, NULL, NULL);
//
//    trie_symbl_t *inserted_seqs[5000][2];
//    for (index = 0; index < 2000; index++)
//    {
//        inserted_seqs[index][0] = symbols + (rand() % max_seq);
//        inserted_seqs[index][1] = inserted_seqs[index][0] + (rand() % ((&symbols[test_size * max_seq] - inserted_seqs[index][0]) - 10));
//        trie_insrt_safe(
//            &trie,
//            inserted_seqs[index][0],
//            inserted_seqs[index][1] - inserted_seqs[index][0],
//            tree_alloc, tree_recyl, trie_alloc, trie_init
//        );
//    }
//
//
//
//
//
//    printf("test_trie: \n");
//    trie_t trie;
//    trie_init(&trie, NULL, NULL);
//    char *strs[] = {"hello", "testing", "book", "page", "pages"};
//    unsigned cnt = sizeof(strs)/sizeof(strs[0]), index;
//
//    for (index = 0; index < cnt; index++)
//        trie_insrt_safe(&trie, (trie_symbl_t *)strs[index], strlen(strs[index]), tree_alloc, tree_recyl, trie_alloc, trie_init);
//
//    for (index = 0; index < cnt; index++)
//        if (!trie_query_safe(&trie, (trie_symbl_t *)strs[index], strlen(strs[index])))
//            printf("failed %s\n", strs[index]);
//
//    if (trie_del_safe(&trie, "page", strlen("page"), tree_recyl, trie_recyl))
//        printf("del ok\n");
//    else
//        printf("failed to del.");
//
//    if (trie_query_safe(&trie, "page", strlen("page")))
//        printf("removed failed!\n");
//
//    char *bad_strs[] = {"asdasd", "1223", "as12", "asf3q24"};
//    cnt = sizeof(bad_strs)/sizeof(bad_strs[0]);
//    for (index = 0; index < cnt; index++)
//        if (trie_query_safe(&trie, (trie_symbl_t *)bad_strs[index], strlen(bad_strs[index])))
//            printf("failed %s\n", bad_strs[index]);
//
//    printf("done.\n");
//}

//void test_packed_bits() {
//    #define PACKED_BITS_SIZE_CNT 7
//    #define PACKED_BITS_TEST_SIZE 100
//    packed_bits_t *pbits = &(packed_bits_t){(typeof(pbits->_bits[0])[PACKED_BITS_TEST_SIZE]){}, sizeof((typeof(pbits->_bits[0])[PACKED_BITS_TEST_SIZE]){})};
//    unsigned char rand_values[PACKED_BITS_TEST_SIZE];
//    int cnt;
//    for (cnt = PACKED_BITS_TEST_SIZE; cnt; cnt--) {
//        rand_values[cnt] = rand() & ((1 << 8) - 1);
//        packed_bits_set(pbits, cnt, rand_values[cnt], 8);
//    }
//
//    for (cnt = PACKED_BITS_TEST_SIZE; cnt; cnt--) {
//        if (rand_values[cnt] != packed_bits_get(pbits, cnt, 8))
//            printf("%hhi %hhi\n", rand_values[cnt], packed_bits_get(pbits, cnt, 8));
//            exit(-1);
//    }
//
//}
//int main()
//{
//    test_packed_bits();
//    return 0;
//}
