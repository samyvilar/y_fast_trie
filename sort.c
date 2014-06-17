
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "bits.h"
#include "mem.h"
#include "vector.h"

#define arry_elem_cnt(arr) (sizeof(arr)/sizeof(arr[0]))
#define exp_2(a) (1 << a)



void sort_8(unsigned char *values, unsigned long cnt)
{
    unsigned long freqs[256] = {0}, index;
    for (index = 0; index < cnt; index++)
        freqs[values[index]]++;

    unsigned short symb_id;
    for (symb_id = 0; symb_id < 256; symb_id++)
        memset_1_byte(values, symb_id, freqs[symb_id]);
}

int cmp_bytes(const void *a, const void *b) {return *(unsigned char *)a - *(unsigned char *)b;  }

void test_char_sort()
{
    #define test_char_size 40000000
    //srand(time(NULL));
    unsigned index;
    unsigned char
            *buffer = malloc(2 * sizeof(char) * test_char_size),
            *rand_vals = buffer,
            *pref_sort = buffer + test_char_size;

    for (index = 0; index < test_char_size; index++)
        pref_sort[index] = rand_vals[index] = rand();

    clock_t begin, end;
    begin = clock();
    qsort(rand_vals, test_char_size, sizeof(char), cmp_bytes);
    end = clock();

    double qsort_time = (double)(end - begin) / CLOCKS_PER_SEC;

    begin = clock();
    sort_8(pref_sort, test_char_size);
    end = clock();

    double bit_sort_time = (double)(end - begin) / CLOCKS_PER_SEC;

    if (memcmp(pref_sort, rand_vals, test_char_size * sizeof(char)))
        for (index = 0; index < test_char_size; index++)
            if (pref_sort[index] != rand_vals[index])
            {
                printf("sort_8: %hhi, qsort: %hhi, index: %u\n", pref_sort[index], rand_vals[index], index);
                printf("sort_8 failed!\n");
                break ;
            }

    printf("sort_8: %fs qsort: %fs, factor: %.3f done\n", bit_sort_time, qsort_time, qsort_time/bit_sort_time);
    free(buffer);
}

void sort_16(unsigned short *values, unsigned long cnt)
{
    static unsigned char prefx_ids[2][256], inv_prefx_ids[2][256];
    unsigned short prefix_id, suffix_id, prefx_ids_cnts[2] = {0}, curr;
    unsigned long bit_flds[2][4] = {0}, index;

    for (index = 0; index < cnt; index++)
        switch ((unsigned char)cnt_leadn_zrs_16(curr = values[index])) {
            case 0 ... 7: bit_fld_set_bit(bit_flds[1], *((unsigned char *)&curr)); continue ;
            case 8 ... 16: bit_fld_set_bit(bit_flds[0], curr);
        }

    for (index = 0; index < (sizeof(bit_flds[0])/sizeof(long long)); index++)
        while (((long long *)(&bit_flds[0]))[index])     // while it has reminaing set bits ...
        {
            prefix_id = index * bit_size(long long) + cnt_trlng_zrs_64(((long long *)(&bit_flds[0]))[index]);
            prefx_ids[0][prefix_id] = prefx_ids_cnts[0];
            inv_prefx_ids[0][prefx_ids_cnts[0]++] = prefix_id;
            bit_fld_clr_bit(bit_flds[0], prefix_id);
        }

    for (index = 0; index < (sizeof(bit_flds[1])/sizeof(long long)); index++)
        while (((long long *)(&bit_flds[1]))[index])
        {
            prefix_id = (index * bit_size(long long)) + cnt_trlng_zrs_64(((long long *)(&bit_flds[1]))[index]);
            prefx_ids[1][prefix_id] = prefx_ids_cnts[1];
            inv_prefx_ids[1][prefx_ids_cnts[1]++] = prefix_id;
            bit_fld_clr_bit(bit_flds[1], prefix_id);
        }

    unsigned
        *chr_freqs = calloc(prefx_ids_cnts[0] + (prefx_ids_cnts[1] * 256), sizeof(unsigned)), // alloc all required
        **short_freqs = malloc(prefx_ids_cnts[1] * sizeof(unsigned *)),
        *buffer = chr_freqs + prefx_ids_cnts[0]; // skip those bytes for chr_freq_count ...

    for (index = 0; index < prefx_ids_cnts[1]; (index++, (buffer += 256)))
        short_freqs[index] = buffer;

    for (index = 0; index < cnt; index++)       // second pass sort 8/16 bit symbols ...
        switch ((unsigned char)cnt_leadn_zrs_16(curr = values[index])) {
            case 0 ... 7:  short_freqs[prefx_ids[1][curr >> 8]][curr & 0b11111111]++; continue ;
            case 8 ... 16: chr_freqs[prefx_ids[0][curr]]++;
        }

    index = 0;
    for (prefix_id = 0; prefix_id < prefx_ids_cnts[0]; prefix_id++) // create 8 bit symbols, in order
        while (chr_freqs[prefix_id]--)
            values[index++] = inv_prefx_ids[0][prefix_id];

    for (prefix_id = 0; prefix_id < prefx_ids_cnts[1]; prefix_id++) // craete 16 bit symbols, in order
        for (suffix_id = 0; suffix_id < 256; suffix_id++)
            while (short_freqs[prefix_id][suffix_id]--)
                values[index++] = (inv_prefx_ids[1][prefix_id] << 8) | suffix_id;

    free(short_freqs);
    free(chr_freqs);
}

int cmp_shorts(const void *a, const void *b) {return *(unsigned short *)a - *(unsigned short *)b; }

void test_short_sort()
{
    #define test_short_size 10000000
    //srand(time(NULL));
    unsigned index;
    unsigned short
        *buffer = malloc(2 * sizeof(short) * test_short_size),
        *rand_vals = buffer,
        *pref_sort = buffer + test_short_size;

    for (index = 0; index < test_short_size; index++)
        pref_sort[index] = rand_vals[index] = rand();

    clock_t begin, end;
    begin = clock();
        qsort(rand_vals, test_short_size, sizeof(short), cmp_shorts);
    end = clock();

    double qsort_time = (double)(end - begin) / CLOCKS_PER_SEC;

    begin = clock();
        sort_16(pref_sort, test_short_size);
    end = clock();

    double bit_sort_time = (double)(end - begin) / CLOCKS_PER_SEC;

    if (memcmp(pref_sort, rand_vals, test_short_size * sizeof(short)))
        for (index = 0; index < test_short_size; index++)
            if (pref_sort[index] != rand_vals[index])
            {
                printf("sort_16: %i, qsort: %i, index: %u\n", pref_sort[index], rand_vals[index], index);
                printf("sort_16 failed!\n");
                break ;
            }

    printf("sort_16: %fs qsort: %fs, factor: %.3f done\n", bit_sort_time, qsort_time, qsort_time/bit_sort_time);
    free(buffer);
}

// fast as possible sorting algorithm, assuming x86 arch with bsr, instruction (bit scan reverse)
void sort_32(unsigned *values, unsigned long cnt)
{

}

//#define bit_sort_mag 4
//#define perm_cnt exp_2
//// prefix sorting algorithm, it assumes that all the values share a comming prefix,
//// only the least bit_size bits are used for actually sorting, the values are sorted and the prefix added...
//void prefix_bit_sort(unsigned *values, const int bit_size, unsigned long cnt, const unsigned prefix)
//{
//    if (!(values && cnt))   return ;
//    if (cnt == 1)   { *values |= prefix; return ; }
//
//    if (bit_size <= bit_sort_mag)
//    {
//       unsigned freqs[exp_2(bit_sort_mag)] = {0}, symbl_index, index = 0;
//        for (index = 0; index < cnt; index++)
//            freqs[values[index]]++;
//
//        index = 0;
//        for (symbl_index = 0; (index < cnt) && (symbl_index < exp_2(bit_sort_mag)); symbl_index++)
//            while (freqs[symbl_index]--)
//                values[index++] = prefix | symbl_index;
//        return ;
//    }
//
//    ll_t
//        ll_blck[cnt],
//        *avalb_elem                          = ll_blck,
//        *prefixes[perm_cnt(bit_sort_mag)]    = {NULL},
//        *new_elem;
//
//    unsigned prefix_id;
//    while (cnt--)
//    {
//        prefix_id = values[cnt] >> (bit_size - bit_sort_mag);
//        new_elem  = ll_blk_alloc(avalb_elem);
//
//        ll_init(new_elem, (values[cnt] & ((1 << (bit_size - bit_sort_mag)) - 1)), prefixes[prefix_id]);
//        prefixes[prefix_id] = new_elem;
//    }
//
//    for (prefix_id = 0; prefix_id < perm_cnt(bit_sort_mag); prefix_id++) // go through each prefix
//    {
//        ll_t *prefs = prefixes[prefix_id];
//        for (cnt = 0; prefs; cnt++)
//        {
//            values[cnt] = ll_value(prefs);
//            prefs = ll_next(prefs);
//        }
//
//        prefix_bit_sort(  // all upper bits are the same, so safe to sort the lower bits ...
//            values,
//            bit_size - bit_sort_mag,
//            cnt,
//            prefix | (prefix_id << (bit_size - bit_sort_mag))
//        );
//
//        values += cnt;
//    }
//}
//
//int _cmp_int_(const void *a, const void *b)   { return *(int *)a - *(int *)b; }
//
//void test_prefix_sorting()
//{
//    srand(time(NULL));
//    unsigned int rand_vals[100], index;
//    for (index = 0; index < sizeof(rand_vals)/sizeof(rand_vals[0]); index++)
//        rand_vals[index] = rand();
//
//    unsigned int pref_sort[arry_elem_cnt(rand_vals)];
//    memcpy(pref_sort, rand_vals, sizeof(rand_vals));
//
//    clock_t begin, end;
//
//    begin = clock();
//    qsort(rand_vals, arry_elem_cnt(rand_vals), sizeof(rand_vals[0]), _cmp_int_);
//    end = clock();
//
//    double qsort_time = (double)(end - begin) / CLOCKS_PER_SEC;
//
//    begin = clock();
//    //prefix_bit_sort(pref_sort, sizeof_bts(rand_vals[0]), arry_elem_cnt(rand_vals), 0);
//    sort_32(pref_sort, arry_elem_cnt(rand_vals));
//    end = clock();
//
//    double bit_sort_time = (double)(end - begin) / CLOCKS_PER_SEC;
//
//
//    if (memcmp(pref_sort, rand_vals, sizeof(rand_vals)))
//    {
//        for (index = 0; index < sizeof(rand_vals)/sizeof(rand_vals[0]); index++)
//            printf("pref: %i, qsort: %i\n", pref_sort[index], rand_vals[index]);
//        printf("prefix failed!\n");
//    }
//
//    printf("bit_sort: %fs qsort: %fs done\n", bit_sort_time, qsort_time);
//
//}

//int main()
//{
//    test_prefix_sorting();
//    test_short_sort();
//    test_char_sort();
//    return 0;
//}