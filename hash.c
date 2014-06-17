
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "hash.h"
#include "bits.h"
#include "alloc.h"

#include "mem.h"

#include "timed.h"
#include "tree.h"

alloc_rec_templs(hash_params)

alloc_rec_templs(hash_tbl)

// sets entries with all the non-null pointers, returns entries
hash_tbl_t **hash_tbl_non_empty_entries(hash_tbl_t *hash_tbl, hash_tbl_t **entries, hash_tbl_t *append_entry) {
    if (__builtin_expect(is_null(hash_tbl) || is_null(entries), 0)) return entries;

    typeof(hash_tbl_cnt(hash_tbl)) index, entry_cnt = 0;
    typeof(hash_tbl_entry(hash_tbl, 0)) entry;
    for (index = 0; index < hash_tbl_size(hash_tbl); index++)
        if (hash_tbl_entry_is_not_empty((entry = hash_tbl_entry(hash_tbl, index))))
            entries[entry_cnt++] = entry;

    if (is_not_null(append_entry))
        entries[entry_cnt++] = append_entry;

    return entries;
}

void hash_tbl_clear_entries(hash_tbl_t *hash_tbl, hash_tbl_t **entries, hash_tbl_size_t cnt) {
    if (__builtin_expect(is_null(hash_tbl) || is_null(entries) || !cnt, 0)) return ;

    hash_tbl_t *entry;
    while (cnt--) { // remove all the added entries ...
        hash_t _hash = hash(hash_tbl, hash_tbl_entry_key(entries[cnt]));
        if (hash_tbl_entry_is_not_empty(entry = hash_tbl_entry(hash_tbl, _hash))) {
            if (hash_tbl_entry_is_sub_tbl(entry)) {
                free(hash_tbl_entries(entry));
                hash_params_recl(hash_tbl_params(entry));
                hash_tbl_recl(entry);
            }
            hash_tbl_set_entry(hash_tbl, _hash, NULL);
        }
    }
}

#define hash_tbl_clear_all_entries(hash_tbl) \
    memset(hash_tbl_entries(hash_tbl), 0, sizeof(hash_tbl_entries(hash_tbl)[0]) * hash_tbl_size(hash_tbl))

#define square(value) ((value) * (value))
#define doubl(value) ((value) * 2)

#define calc_new_capcity(old_capc) (2 * (old_capc)) // double the previous capacity ...
#define hash_tbl_new_capacity(hash_tbl) calc_new_capcity(hash_tbl_capacity(hash_tbl))

#define calc_new_size(old_cpc) (4 * (old_cpc) * (old_cpc))
#define hash_tbl_new_size(hash_tbl) (4 * hash_tbl_capacity(hash_tbl) * hash_tbl_capacity(hash_tbl)) // -1

#define hash_tbl_sub_tbls_max_bucket_cnt(hash_tbl) (((32 * square(hash_tbl_size(hash_tbl))) / hash_tbl_cnt(hash_tbl)) + (4 * hash_tbl_size(hash_tbl)))


#define hash_tbl_entries_recl free
#define hash_tbl_entries_alloc(entry_cnt) malloc((entry_cnt) * sizeof(hash_tbl_t *))
#define hash_tbl_entries_init(entries, cnt) memset((entries), 0, (sizeof(hash_tbl_t *) * (cnt)))

#define hash_tbl_stbl_lvl_insert(lvl) ((hash_tbl_cnt(lvl) + 1) < hash_tbl_cnt(lvl))
#define hash_tbl_stbl_lvl_expand(hash_tbl, lvl) (\
    (hash_tbl_sub_tbls_bucket_cnt(hash_tbl) + ((8 * square(hash_tbl_cnt(lvl))) - (4 * hash_tbl_cnt(lvl))))\
        <= hash_tbl_sub_tbls_max_bucket_cnt(hash_tbl) \
)

void hash_tbl_rehash(hash_tbl_t *hash_tbl, hash_tbl_t **entries, hash_tbl_size_t cnt) { // rehashes a subtable by creating a new hashing function and inserting until no collisions occur ...
    if (__builtin_expect(is_null(hash_tbl) || is_null(entries) || !cnt, 0)) return ;

    hash_params_rand_init(hash_tbl_params(hash_tbl));

    hash_tbl_size_t index;
    hash_tbl_clear_all_entries(hash_tbl); // clear all entries ...
    for (index = 0; index < cnt; index++) {
        hash_t _hash = hash(hash_tbl, hash_tbl_entry_key(entries[index]));
        hash_tbl_t *entry = hash_tbl_entry(hash_tbl, _hash);

        if (hash_tbl_entry_is_not_empty(entry)) { // collision.
            hash_tbl_clear_entries(hash_tbl, entries, index);
            hash_params_rand_init(hash_tbl_params(hash_tbl)); // get a new random function ...
            index = -1; // reset.
        }
        else
            hash_tbl_set_entry(hash_tbl, _hash, entries[index]);
    }
}

void hash_tbl_expand_sub_lvl(hash_tbl_t *hash_tbl, hash_tbl_t *append_entry) {
    if (__builtin_expect(is_null(hash_tbl), 0)) return ;

    typeof(hash_tbl_entry(hash_tbl, 0)) entries[hash_tbl_cnt(hash_tbl) + bool(append_entry)];
    hash_tbl_non_empty_entries(hash_tbl, entries, append_entry);

    if (is_not_null(append_entry)) hash_tbl_inc_cnt(hash_tbl);

    hash_tbl_entries_recl(hash_tbl_entries(hash_tbl));

    hash_tbl_rehash(
        hash_tbl_sub_tbl_init(
            hash_tbl,
            hash_tbl_new_size(hash_tbl),
            hash_tbl_new_capacity(hash_tbl),
            hash_tbl_entries_alloc(hash_tbl_new_size(hash_tbl)),
            hash_tbl_cnt(hash_tbl),
            hash_params_rand_init(hash_tbl_params(hash_tbl))
        ),
        entries,
        hash_tbl_cnt(hash_tbl)
    );
}

#define HASH_TBL_SUB_TBL_PREV_SIZE 1
#define hash_tbl_init_sub_lvl(hash_tbl, sub_level, sub_level_hash, new_entry) ({\
    typeof(hash_tbl_entry(hash_tbl, 0)) sub_entries[2] = {(sub_level), (new_entry)};\
    \
    hash_tbl_set_entry(\
        hash_tbl, sub_level_hash,\
        hash_tbl_sub_tbl_init(\
            hash_tbl_alloc(),\
            calc_new_size(HASH_TBL_SUB_TBL_PREV_SIZE),\
            calc_new_capcity(2 * HASH_TBL_SUB_TBL_PREV_SIZE),\
            hash_tbl_entries_alloc(calc_new_size(HASH_TBL_SUB_TBL_PREV_SIZE)),\
            array_cnt(sub_entries),\
            hash_params_alloc()\
        )\
    );\
    \
    hash_tbl_sub_tbls_upd_bucket_cnt(hash_tbl, hash_tbl_size(hash_tbl_entry(hash_tbl, sub_level_hash)));\
    hash_tbl_rehash(hash_tbl_entry(hash_tbl, sub_level_hash), sub_entries, hash_tbl_cnt(hash_tbl_entry(hash_tbl, sub_level_hash)));\
})

void dyn_perf_hash_tbl_rebuild(hash_tbl_t *hash_tbl, hash_tbl_t *new_entry) { // rebuild the entire table with a larger size.
    if (__builtin_expect(is_null(hash_tbl), 0)) return ;

    // copy all the non-empty entries ...
    typeof(hash_tbl_entry(hash_tbl, 0)) entries[hash_tbl_cnt(hash_tbl) + bool(new_entry)];
    typeof(hash_tbl_size(hash_tbl)) index, entry_index = 0;

    for (index = 0; index < hash_tbl_size(hash_tbl); index++) {
        typeof(entries[0]) entry = hash_tbl_entry(hash_tbl, index);
        if (hash_tbl_entry_is_empty(entry)) continue ;

        if (hash_tbl_entry_is_sub_tbl(entry)) {
            hash_tbl_non_empty_entries(entry, &entries[entry_index], NULL);
            entry_index += hash_tbl_cnt(entry);

            hash_params_recl(hash_tbl_params(entry)); // recycle sub hash table ...
            hash_tbl_entries_recl(hash_tbl_entries(entry));
            hash_tbl_recl(entry);
        } else
            entries[entry_index++] = entry;
    }

    if (is_not_null(new_entry)) entries[entry_index++] = new_entry;

    hash_tbl_entries_recl(hash_tbl_entries(hash_tbl));


    dyn_perf_hash_tbl_init(
        hash_tbl,
        doubl(hash_tbl_size(hash_tbl)), // double the previous size ...
        0, 0,
        hash_tbl_entries_init(
           hash_tbl_entries_alloc(doubl(hash_tbl_size(hash_tbl)) + 1),
           doubl(hash_tbl_size(hash_tbl)) + 1
        ),
        hash_params_rand_init(hash_tbl_params(hash_tbl))
    );

    for (index = 0; index < entry_index; index++) { // re-insert all entries ...
        hash_tbl_inc_cnt(hash_tbl);
        hash_t _hash = hash(hash_tbl, hash_tbl_entry_key(entries[index]));
        typeof(hash_tbl_entry(hash_tbl, 0)) entry = hash_tbl_entry(hash_tbl, _hash);

        if (hash_tbl_entry_is_empty(entry))   // no collision
            hash_tbl_set_entry(hash_tbl, _hash, entries[index]);
        else if (hash_tbl_entry_is_not_sub_tbl(entry)) {  // initial level 1 collision.
            if ((hash_tbl_sub_tbls_bucket_cnt(hash_tbl) + calc_new_size(HASH_TBL_SUB_TBL_PREV_SIZE)) <= hash_tbl_sub_tbls_max_bucket_cnt(hash_tbl)) // convert entry into sub table ...
                hash_tbl_init_sub_lvl(hash_tbl, entry, _hash, entries[index]);
            else
                goto unstable_level_1;
        } else if (hash_tbl_stbl_lvl_insert(entry)) { // level 1 collision, stable level 2
            hash_tbl_inc_cnt(entry);
            _hash = hash(entry, hash_tbl_entry_key(entries[index]));

            if (hash_tbl_entry_is_empty(hash_tbl_entry(entry, _hash)))  // no collision on level 2
                hash_tbl_set_entry(entry, _hash, entries[index]);
            else {// level 2 collision, look for new hash_function ...
                typeof(entry) entries[hash_tbl_cnt(hash_tbl)];
                hash_tbl_rehash(entry, hash_tbl_non_empty_entries(entry, entries, entries[index]), hash_tbl_cnt(entry));
            }
        } else if (hash_tbl_stbl_lvl_expand(hash_tbl, entry)) { // unstable level 2 collision, but stable level 1, expand level 2
            hash_tbl_size_t prev_size = hash_tbl_size(entry);
            hash_tbl_expand_sub_lvl(entry, entries[index]);
            hash_tbl_sub_tbls_upd_bucket_cnt(hash_tbl, hash_tbl_size(entry) - prev_size);
        }
        else { // non-stable level 1 collision, need to rebuild the entire table ... (hopefully this should be really rare!!!!)
            unstable_level_1:

            hash_tbl_clear_entries(hash_tbl, entries, index); // clear all previous inserted entries ...
            dyn_perf_hash_tbl_init(
                hash_tbl,
                hash_tbl_size(hash_tbl), 0, 0, hash_tbl_entries(hash_tbl),
                hash_params_rand_init(hash_tbl_params(hash_tbl)) // create a new random function ...
            );
            index = -1; // restart search ...
        }
    }
}

/*
    A Dynamic perfect hash table uses a two level hash table, where the first level is of size M which
    is (1 + c) * n, where n is the number of 'expected' entries meant to hold (its capacity)
    and c is some constant greater than 0,
    another way of viewing it is: M = load_factor * n, where load_factor is any value equal or greater than 1,
    the greater the load_factor the more slack, and reduces the number collisions on the first level,
    and the need for subtables., but consumes more space.

    assuming each subtable is of size s, meant to hold m entries, where s = 2 * m * max((m - 1), 1)
    then when to insert look for an entry:
    if entry is:
        - if empty we simply set (key, value) pair.
        - else if not a hashtable and keys match, simply update the value.
        - else if not a hashtable. // collision on non-hash-table entry, we need to create a new one.
            // check if all the the values are evenly distributed.
                //  (we add 3 since the previous hashtable was of size 1, and the new size will be 4) and (n + 1) since we are adding a new element.
                if (sum of sizeof of all sub_hash_tables) + 3 <= (32*sizeof(hash_table)^2 / (hash_table->cnt + 1)) + 4*sizeof(hash_table)
                    create new hash_table of size 2 * m = 2, s = 2 * m * max(2 - 1, 1), 2 * 2 * 1, or 4 by looking for an injective hash func for the two new keys.
                    (insert the 2 key value pairs)
                else // the subtables are not evenly distributed ...
                    rebuild the entire table:
                        - collect all key-value pairs.
                        - recycle the entire table and sub_tables.
                        - continously try to build a new hashtable meant to hold 2*(n + 1) entries, so size M = load_factor * max(count, 4)
                            until the values are evenly distributed
    else: // it is hash-table and keys match, look for withing sub-table.
         if sub_table entry is:
            - if empty we simply insert key/value pair.
            - else if keys match update value.
            - else if sub_table->_cnt + 1 <= m(j) // where m(j) is the max number of values expected or max capacity of this table.
            - else if (sum of sizeof of all sub_hash_tables) + (8*sub_table->cnt^2 - 3*sub_table->cnt) // sj = (8*mj^2 - 4*mj), subtract mj to account for previous size.
            <= (32*sizeof(hash_table)^2 / (hash_table->cnt + 1)) + 4*sizeof(hash_table)
                - collect all key-value pairs
                - recycle hash_table
                - create new hash_table sub_table with size 8*sub_table->cnt^2 - 4*sub_table->cnt, capacity 2 * sub_table->cnt * (sub_table->cnt - 1)
                - locate new injective hash_func on the values, insert the new values.
            - else
                rebuild entire hash_table.
 */

#define hash_tbl_insert_is_update(entry, key) (hash_tbl_entry_is_not_sub_tbl(entry) && (hash_tbl_entry_key(entry) == key))

#define hash_tbl_entry_new(key, value) hash_tbl_entry_init(hash_tbl_entry_alloc(), key, value)

// either inserts or updates an entry ...
void dyn_perf_hash_insert(hash_tbl_t *hash_tbl, hash_tbl_entry_key_t key, void *value) {
    if (__builtin_expect(is_null(hash_tbl), 0)) return;

    hash_t
        _hash = hash(hash_tbl, key);

    typeof(hash_tbl_entry(hash_tbl, 0)) entry = hash_tbl_entry(hash_tbl, _hash);

    if (hash_tbl_entry_is_empty(entry)) {
        if ((hash_tbl_cnt(hash_tbl) + 1) < hash_tbl_size(hash_tbl)) {
            hash_tbl_inc_cnt(hash_tbl);
            hash_tbl_set_entry(hash_tbl, _hash, hash_tbl_entry_new(key, value));
        } else
            dyn_perf_hash_tbl_rebuild(hash_tbl, hash_tbl_entry_new(key, value));
    } else if (hash_tbl_insert_is_update(entry, key)) {
        hash_tbl_entry_set_value(hash_tbl_entry(hash_tbl, _hash), value);
    } else if (hash_tbl_entry_is_not_sub_tbl(entry)) { // initial level 1 collision.
        hash_tbl_inc_cnt(hash_tbl);
        if ((hash_tbl_cnt(hash_tbl) < hash_tbl_size(hash_tbl))
            && ((hash_tbl_sub_tbls_bucket_cnt(hash_tbl) + calc_new_size(HASH_TBL_SUB_TBL_PREV_SIZE)) <= hash_tbl_sub_tbls_max_bucket_cnt(hash_tbl))) // convert entry into sub table ...
            hash_tbl_init_sub_lvl(hash_tbl, entry, _hash, hash_tbl_entry_new(key, value));
        else
            dyn_perf_hash_tbl_rebuild(hash_tbl, hash_tbl_entry_new(key, value));
    } else { // entry is sub table, we may need to insert/update/rehash sub tbl/expand sub_tbl/rebuild entire table.
        _hash = hash(entry, key); // get hash of subtable ..
        typeof(entry) sub_entry = hash_tbl_entry(entry, _hash);

        if (hash_tbl_entry_is_empty(sub_entry)) {// subtable entry is empty so just insert it.
            if (((hash_tbl_cnt(hash_tbl) + 1) < hash_tbl_size(hash_tbl))) {
                hash_tbl_set_entry(entry, _hash, hash_tbl_entry_new(key, value));
                hash_tbl_inc_cnt(hash_tbl); // increment global count
                hash_tbl_inc_cnt(entry); // increment sub table count.
            } else dyn_perf_hash_tbl_rebuild(hash_tbl, hash_tbl_entry_new(key, value));
        } else if (hash_tbl_entry_key(sub_entry) == key) { // keys match on subtable so update subtable entry ...
            hash_tbl_entry_set_value(sub_entry, value);
        } else if ((hash_tbl_cnt(hash_tbl) + 1) >= hash_tbl_size(hash_tbl)) {
            dyn_perf_hash_tbl_rebuild(hash_tbl, hash_tbl_entry_new(key, value));
        } else if ((hash_tbl_cnt(entry) + 1) < hash_tbl_capacity(entry)) {// still enough space just find a new hashing function
            hash_tbl_inc_cnt(hash_tbl);
            hash_tbl_inc_cnt(entry);
            typeof(hash_tbl_entry(hash_tbl, 0)) entries[hash_tbl_cnt(entry)];

            hash_tbl_rehash(
                entry,
                hash_tbl_non_empty_entries(entry, entries, hash_tbl_entry_new(key, value)),
                hash_tbl_cnt(entry)
            );

        } else if (hash_tbl_stbl_lvl_expand(hash_tbl, entry)) { // safe to expand sub table ..
            hash_tbl_inc_cnt(hash_tbl);
            hash_tbl_size_t prev_size = hash_tbl_size(entry);
            hash_tbl_expand_sub_lvl(entry, hash_tbl_entry_new(key, value));
            hash_tbl_sub_tbls_upd_bucket_cnt(hash_tbl, hash_tbl_size(entry) - prev_size);
        } else
            dyn_perf_hash_tbl_rebuild(hash_tbl, hash_tbl_entry_new(key, value));
    }
}

void *dyn_perf_hash_del(hash_tbl_t *hash_tbl, hash_tbl_entry_key_t key) {
    if (__builtin_expect(is_null(hash_tbl), 0))
        return NULL;

    return NULL;

}
void *dyn_perf_hash_query(hash_tbl_t *hash_tbl, hash_tbl_entry_key_t key) { // returns value or NULL if not present ...
    if (__builtin_expect(is_null(hash_tbl), 0)) return NULL;

    typeof(hash_tbl_entry(hash_tbl, 0)) entry = hash_tbl_entry(hash_tbl, hash(hash_tbl, key));

    if (hash_tbl_entry_is_empty(entry)) return NULL;

    if (hash_tbl_entry_is_not_sub_tbl(entry))
        return (key == hash_tbl_entry_key(entry)) ? hash_tbl_entry_value(entry) : NULL;

    entry = hash_tbl_entry(entry, hash(entry, key));

    return (hash_tbl_entry_is_not_empty(entry) && (hash_tbl_entry_key(entry) == key)) ? hash_tbl_entry_value(entry) : NULL;
}


#define hash_params_size(params) (__builtin_expect(is_null(params), 0) ? 0 : sizeof(*params))
#define hash_tbl_entries_size(hash_tbl) (\
    __builtin_expect(is_null(hash_tbl) && is_null(hash_tbl_entries(hash_tbl)), 0) \
        ? 0 : (sizeof(hash_tbl_entry(hash_tbl, 0)) * hash_tbl_size(hash_tbl))\
)

hash_tbl_size_t hash_tbl_byte_size(hash_tbl_t *hash_tbl) { // returns the number of bytes the hash_tbl is currently occupying, ecluding the pointer itself.
    if (__builtin_expect(is_null(hash_tbl), 0)) return 0;

    hash_tbl_size_t
        cnt = sizeof(*hash_tbl) // space for the object required for managment ...
            + hash_params_size(hash_tbl_params(hash_tbl)) // space required for hash function ..
            + hash_tbl_entries_size(hash_tbl),  // space required for all possible entries ...
            index;

    for (index = 0; index < hash_tbl_size(hash_tbl); index++) {
        typeof(hash_tbl_entry(hash_tbl, 0)) entry = hash_tbl_entry(hash_tbl, index);
        if (hash_tbl_entry_is_empty(entry))
            continue ;

        if (hash_tbl_entry_is_sub_tbl(entry)) // if entry is subtrable account for it ...
            cnt += hash_params_size(hash_tbl_params(entry)) // space required for hash function ...
                + hash_tbl_entries_size(entry) // space required for all possible entries ...
                + sizeof(*entry) * hash_tbl_cnt(entry); // space required for each entry ...

        cnt += sizeof(*entry);
    }

    return cnt;

}
void test_dyn_perf_hash_tbl() {
    #define DYN_PERF_HASH_TBL_TEST_SIZE 1000000
    #define DYN_PERF_HASH_TBL_SIZE 128

    hash_tbl_t *hash_tbl = dyn_perf_hash_tbl_init(
        hash_tbl_alloc(),
        DYN_PERF_HASH_TBL_SIZE,
        0,
        0,
        memset(hash_tbl_entries_alloc(DYN_PERF_HASH_TBL_SIZE + 1), 0, (DYN_PERF_HASH_TBL_SIZE + 1) * sizeof(typeof(hash_tbl_entry(hash_tbl, 0)))),
        hash_params_rand_init(hash_params_alloc())
    );
    struct key_val_t {hash_tbl_entry_key_t key; void *value;} *entries = malloc(DYN_PERF_HASH_TBL_TEST_SIZE * sizeof(*entries));

    hash_tbl_size_t index;
    for (index = 0; index < DYN_PERF_HASH_TBL_TEST_SIZE; index++)
        entries[index] = (typeof(entries[index])){.key = rand(), .value = (void *)rand()};

    void *temp;
    #define test_dyn_perf_hash_insert() ({\
        for (index = 0; index < DYN_PERF_HASH_TBL_TEST_SIZE; index++) {\
            dyn_perf_hash_insert(hash_tbl, entries[index].key, entries[index].value);\
                /*if (__builtin_expect((temp = dyn_perf_hash_query(hash_tbl, entries[index].key)) != entries[index].value, 0))\
                    printf("Bad Insert index: %llu exp: %p got: %p", index, entries[index].value, temp), exit(-1);*/\
        }})
    double insert_time = timed(test_dyn_perf_hash_insert);

    #define test_dyn_perf_hash_query() ({\
        for (index = 0; index < DYN_PERF_HASH_TBL_TEST_SIZE; index++) {\
            if (__builtin_expect((temp = dyn_perf_hash_query(hash_tbl, entries[index].key)) != entries[index].value, 0))\
                printf("Bad Query index: %llu exp: %p got: %p", index, entries[index].value, temp), exit(-1);\
        }})
    double query_time = timed(test_dyn_perf_hash_query);

    printf(
        "\ndyn_perf_hash_tbl test_size: %i(key/value pairs), %lu(bytes) \n"
        "tbl_size: %u, total_buckets: %u(buckets), %u(total bytes) efficiency: %.4f\n"
        "cnt: %u, main_lvl_capc: %.4f sub_lbl_capc %.4f insert_time: %.4fs query_time: %.4fs\n",
        DYN_PERF_HASH_TBL_TEST_SIZE, ((sizeof(hash_tbl_entry_key_t) + sizeof(hash_tbl_entry_value_t)) * DYN_PERF_HASH_TBL_TEST_SIZE),
        hash_tbl_size(hash_tbl),
        hash_tbl_sub_tbls_bucket_cnt(hash_tbl) + hash_tbl_size(hash_tbl),
        hash_tbl_byte_size(hash_tbl),
        (double)((sizeof(hash_tbl_entry_key_t) + sizeof(hash_tbl_entry_value_t)) * DYN_PERF_HASH_TBL_TEST_SIZE)/((double)hash_tbl_byte_size(hash_tbl)),
        hash_tbl_cnt(hash_tbl),
        (double)hash_tbl_cnt(hash_tbl)/(double)hash_tbl_size(hash_tbl),
        (((double)hash_tbl_sub_tbls_bucket_cnt(hash_tbl))/(double)hash_tbl_sub_tbls_max_bucket_cnt(hash_tbl)),
        insert_time,
        query_time);

    free(entries);
}


int hash_main()
{
    test_tree();
    test_dyn_perf_hash_tbl();

    return 0;
}
