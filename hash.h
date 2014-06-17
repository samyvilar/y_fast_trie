#ifndef __HASH__
#define __HASH__

#define MAX_PRIME_32 4294967291U
#define MAX_PRIME_64 18446744073709551557LLU

#define max_prime(var) ({                       \
    typeof(var) prime;                          \
    switch (sizeof(var)) {                      \
        case 4: prime = MAX_PRIME_32; break;    \
        case 8: prime = MAX_PRIME_64;           \
    } prime;                                    \
})


#include "bits.h"
#include "mem.h"

#define hash_tbl_entry_value_t void *
#define hash_tbl_entry_key_t unsigned

#define hash_t hash_tbl_entry_key_t
#define hash_tbl_size_t hash_t
#define hash_tbl_bit_size_t hash_t

// the radom coefficient must be small enough so as to prevent overflow ...
#define rand_params(params) ({\
    unsigned *_bytes = (void *)(&(params));\
    unsigned index;\
    for (index = 0; index < sizeof(params)/sizeof(_bytes[0]); index++) {\
        switch(sizeof(params[0])) {\
            case 8: _bytes[7] = (unsigned)(rand() & 255); case 7: _bytes[6] = (unsigned)(rand() & 255);\
            case 6: _bytes[5] = (unsigned)(rand() & 255); case 5: _bytes[4] = (unsigned)(rand() & 255);\
            case 4: _bytes[3] = (unsigned)(rand() & 255); case 3: _bytes[2] = (unsigned)(rand() & 255);\
            case 2: _bytes[1] = (unsigned)(rand() & 255); case 1: _bytes[0] = (unsigned)(rand() & 255);\
        } params[index] = ((params[index] % max_prime(params[0])) | (typeof(params[0]))1);\
    }\
    (params);}) // random odd positive integer ...

#define LOAD_FACTOR 1
//#define HASH_PARAMS_CNT 2

typedef struct hash_params_t {
    union {
        struct hash_params_t *_next;
        struct {
            hash_t _coefs[1];
            hash_tbl_size_t _cnt; // number of elements inserted.
        };
    };
//    union {
//        unsigned _capacity; // capacitity of subtables ....
//        unsigned _bucket_cnt; //for main hash_tbl, the total number of buckets ie the sum of all the sub_tbls ..
//    };
} hash_params_t;

// counts the total number of buckets

#define perf_sqrt_pow_2(a) (1 << (cnt_trlng_zrs(a) >> 1)) // assumes a is perfect square (4, 16, 64, 256, 1024 ...)
#define hash_tbl_capacity(hash_tbl) perf_sqrt_pow_2(hash_tbl_size(hash_tbl)) // (hash_tbl_params(hash_tbl)->_capacity)
#define hash_tbl_set_capacity(hash_tbl, capc) //(hash_tbl_capacity(hash_tbl) = (capc))


//#define hash_param_scaled(a) ((a)  % max_prime(a))
//#define hash_param_scaled(a) (a)

//#define hash_params_init(hash_param, rand_a) (*(hash_param) = (hash_params_t){.params[0] = hash_param_scaled(rand_a), .params[1] = hash_param_scaled(rand_b), ._cnt = 0})
//#define hash_params_rand_init(hash_param) ({hash_params_t *temp = hash_param; hash_params_init(temp, rand(), rand()); temp;})
#define hash_params_coefs(hash_param) ((hash_param)->_coefs)
#define hash_params_coef(hash_param, index) (hash_params_coefs(hash_param)[index])

#define hash_params_init(hash_param, values) memcpy(hash_params_coefs(hash_param), values, sizeof(hash_params_coefs(hash_param)))

#define hash_params_rand_init(hash_param) ({\
    hash_params_t *__temp__ = hash_param; \
    rand_params(hash_params_coefs(__temp__)); \
    __temp__;})

// assumes that the tbl size is of power of 2
#define hash(hash_tbl, k) ({\
    hash_t __hash__ = 0; \
    hash_params_t *_hash_params_ = hash_tbl_params(hash_tbl);\
    switch(array_cnt(hash_params_coefs(_hash_params_))) {\
        case 2: __hash__ = hash_params_coef(_hash_params_, 1) >> (cnt_leadn_zrs(hash_tbl_size(hash_tbl)) + 1);\
        case 1: __hash__ += (hash_params_coef(_hash_params_, 0) * (k));\
    } __hash__ >> (cnt_leadn_zrs(hash_tbl_size(hash_tbl)) + 1); })


typedef struct hash_tbl_t {
    union {
        struct hash_tbl_t *_next;           // for the alloc and recl ...

        struct hash_tbl_t **_entries;       // for a hash_tbl object, pointer to sequence of slots..
        hash_tbl_entry_value_t *_value;     // for a hash_tbl entry, the actual value corresponding to _key.

        unsigned word_t *_buckets_cnt;      // to be used by the first level to keep track of the number of buckets ...
    };

    union {
        hash_tbl_entry_key_t _key; // for entries ...
        hash_tbl_size_t _size;     // for hash_table, the number of slots.
    };

    struct hash_params_t *_params; // NULL if hash_tbl is entry, otherwise this entry is a hash_tbl itself and this is holds its hash func as well as any other properties such as _cnt
} hash_tbl_t;

#define hash_tbl_sub_tbls_bucket_cnt(hash_tbl) ((hash_tbl)->_buckets_cnt[hash_tbl_size(hash_tbl)])
#define hash_tbl_sub_tbls_set_bucket_cnt(hash_tbl, v) (hash_tbl_sub_tbls_bucket_cnt(hash_tbl) = (unsigned word_t)(v))
#define hash_tbl_sub_tbls_upd_bucket_cnt(hash_tbl, upd_val) (hash_tbl_sub_tbls_bucket_cnt(hash_tbl) += (upd_val))


// perfect hash func assuming table size is a power of 2
// hash_tbl: ***********************************************************************************************************
#define hash_tbl_init(_hash_tbl_macro_param_, size, cnt, entries, _params_macro_param_) ({\
    hash_tbl_t *_hash_tbl_ = (_hash_tbl_macro_param_);\
    hash_params_t *_params_ = (_params_macro_param_);\
    *_hash_tbl_ = (hash_tbl_t){._size = (size), ._entries = (entries), ._params = _params_};\
     hash_tbl_set_cnt(_hash_tbl_, cnt); \
     _hash_tbl_;})

#define hash_tbl_sub_tbl_init(hash_tbl, size, capacity, entries, cnt, params) ({\
    hash_tbl_t *__hash_tbl__ = (hash_tbl);\
    hash_params_t *__params__ = (params);\
    *__hash_tbl__ = (hash_tbl_t){._size = (size), ._entries = (entries), ._params = __params__}; \
     hash_tbl_set_capacity(__hash_tbl__, capacity);\
     hash_tbl_set_cnt(__hash_tbl__ , cnt);\
     __hash_tbl__;})

#define dyn_perf_hash_tbl_init(hash_tbl, size, cnt, sub_tbl_bucket_cnt, entries, params)({\
    hash_tbl_t *__hash_tbl__ = (hash_tbl);\
    hash_params_t *__params__ = (params); \
    *__hash_tbl__ = (hash_tbl_t){._size = (size), ._entries = (entries), ._params = __params__};\
    hash_tbl_set_cnt(__hash_tbl__, cnt); \
    hash_tbl_sub_tbls_set_bucket_cnt(__hash_tbl__, sub_tbl_bucket_cnt);\
    __hash_tbl__;})

#define hash_tbl_size(hash_tbl) ((hash_tbl)->_size)
#define hash_tbl_set_size(hash_tbl, size) (hash_tbl_size(hash_tbl) = (size))

#define hash_tbl_entries(hash_tbl) ((hash_tbl)->_entries)
#define hash_tbl_set_entries(hash_tbl, entries) (hash_tbl_entries(hash_tbl) = (entries))

#define hash_tbl_params(hash_tbl) ((hash_tbl)->_params)
#define hash_tbl_set_params(hash_tbl, params) (hash_tbl_params(hash_tbl) = (params))

#define hash_tbl_entry(hash_tbl, index) (hash_tbl_entries(hash_tbl)[(index)])
#define hash_tbl_set_entry(hash_tbl, index, entry) (hash_tbl_entry(hash_tbl, index) = (entry))

#define hash_tbl_cnt(hash_tbl) (hash_tbl_params(hash_tbl)->_cnt)
#define hash_tbl_set_cnt(hash_tbl, c) (hash_tbl_cnt(hash_tbl) = (c))
#define hash_tbl_inc_cnt(hash_tbl) (++hash_tbl_cnt(hash_tbl))
#define hash_tbl_dec_cnt(hash_tbl) (--hash_tbl_cnt(hash_tbl))

// hash_tbl_entry: *****************************************************************************************************
#define hash_tbl_entry_key(entry) ((entry)->_key)
#define hash_tbl_entry_set_key(entry, k) (hash_tbl_entry_key(entry) = (k))

#define hash_tbl_entry_value(entry) ((entry)->_value)
#define hash_tbl_entry_set_value(entry, v) (hash_tbl_entry_value(entry) = (v))

#define hash_tbl_entry_init_sub_tbl(entry, bitsize, entries, params)

#define hash_tbl_entry_is_empty(entry) is_null(entry)
#define hash_tbl_entry_is_not_empty(entry) is_not_null(entry)

#define hash_tbl_entry_is_sub_tbl(entry) is_not_null(hash_tbl_params(entry))
#define hash_tbl_entry_is_not_sub_tbl(entry) (is_null(hash_tbl_params(entry)))

#define hash_tbl_entry_alloc hash_tbl_alloc
#define hash_tbl_entry_recl hash_tbl_recl
#define hash_tbl_entry_init(entry, key, value) ({hash_tbl_t *_entry = entry; *_entry = (hash_tbl_t){._key = key, ._value = value, ._params = NULL}; _entry;})


void dyn_perf_hash_insert(hash_tbl_t *, hash_tbl_entry_key_t key, void *value);
void *dyn_perf_hash_del(hash_tbl_t *, hash_tbl_entry_key_t key);
void *dyn_perf_hash_query(hash_tbl_t *, hash_tbl_entry_key_t key);

#endif