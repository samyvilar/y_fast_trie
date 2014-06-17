

#ifndef __VECTOR_H_
#define __VECTOR_H_

#include <emmintrin.h>
#include <pmmintrin.h>

#include <limits.h>


// instrinsics for vector type sse, avx ...
#if defined (__AVX__)
    #define vector_f __m256d
    #define vector_load             _mm256_lddqu_si256
    #define vector_load_align       _mm256_load_si256 // _mm256_stream_load_si256

    #define vector_broad_cast_f(a) ({                       \
        vector_f _r;                                        \
        switch (sizeof(a)) {                                \
            case 4: _r = _mm256_set1_ps(a); break ;         \
            case 8: _r = _mm256_set1_pd(a); } _r; })

    #define vector_add_f(a, b, _t) ({                       \
        vector_f _r;                                        \
        switch (sizeof(_t)) {                               \
            case 4: _r = _mm256_add_ps(a, b); break ;       \
            case 8: _r = _mm256_add_pd(a, b); } _r; })

    #define vector_sub_f(a, b, _t)  ({                      \
        vector_t _r;                                        \
        switch (sizeof(_t)) {                               \
            case 4: _r = _mm256_sub_ps(a, b); break ;       \
            case 8: _r = _mm256_sub_pd(a, b); } _r; })

    #define vector_mul_f(a, b, _t) ({                       \
        vector_f _r;                                        \
        switch (sizeof(_t)) {                               \
            case 4: _r = _mm256_mul_ps(a, b); break ;       \
            case 8: _r = _mm256_mul_pd(a, b); } _r })

    #define vector_div_f(a, b, _t) ({                       \
        vector_f _r;                                        \
        switch (sizeof(_t)) {                               \
            case 4: _r = _mm256_div_ps(a, b); break ;       \
            case 8: _r = _mm256_div_pd(a, b); break ; } _r; })


    #ifdef defined (__AVX2__)
        #define vector_t                __m256i

        #define vector_xor              _mm256_xor_si256
        #define vector_or               _mm256_or_si256
        #define vector_and              _mm256_and_si256

        #define vector_broad_cast_i(a) ({                   \
            vector_t _a;                                    \
            switch (sizeof(a)) {                            \
                case 1: _a = _mm256_set1_epi8(a); break ;   \
                case 2: _a = _mm256_set1_epi16(a); break ;  \
                case 4: _a = _mm256_set1_epi32(a); break ;  \
                case 8: _a = _mm256_set1_epi64x(a); break ; } _a;})

        #define vector_are_eq(a, b)     _mm256_testz_si256(vector_xor((a), (b)), _mm256_set1_epi32(-1))
    #else
        #define vector_f __m256d

        #define vector_or               _mm256_or_pd
        #define vector_xor              _mm256_xor_pd
        #define vector_and              _mm256_and_pd

        #define vector_are_eq(a, b)     _mm256_testz_si256(vector_xor((a), (b)), _mm256_set1_epi32(-1))
    #endif
#elif defined (__SSE2__)
    #define vector_t    __m128i
    #define vector_f    __m128f


    #define vector_load_align _mm_load_si128 // _mm_stream_load_si128 seems slower ...
    #if defined (__SSE3__)
        #define vector_load _mm_lddqu_si128
    #else
        #define vector_load _mm_loadu_si128
    #endif

    #define vector_store_algin  _mm_store_si128 // _mm_stream_si128 seems slower
    #define vector_store        _mm_storeu_si128

    #define vector_xor  _mm_xor_si128
    #define vector_or   _mm_or_si128
    #define vector_and  _mm_and_si128

    #define vector_broad_cast_f(a) ({                       \
        vector_f _r;                                        \
        switch (sizeof(a)) {                                \
            case 4: _r = _mm_set1_ps(a); break ;            \
            case 8: _r = _mm_set1_pd(a); } _r; })

    #define vector_add_f(a, b, _t) ({                       \
        vector_f _r;                                        \
        switch (sizeof(_t)) {                               \
            case 4: _r = _mm_add_ps(a, b); break ;          \
            case 8: _r = _mm_add_pd(a, b); } _r; })

    #define vector_sub_f(a, b, _t)  ({                      \
        vector_t _r;                                        \
        switch (sizeof(_t)) {                               \
            case 4: _r = _mm_sub_ps(a, b); break ;          \
            case 8: _r = _mm_sub_pd(a, b); } _r; })

    #define vector_mul_f(a, b, _t) ({                       \
        vector_f _r;                                        \
        switch (sizeof(_t)) {                               \
            case 4: _r = _mm_mul_ps(a, b); break ;          \
            case 8: _r = _mm_mul_pd(a, b); } _r })

    #define vector_div_f(a, b, _t) ({                       \
        vector_f _r;                                        \
        switch (sizeof(_t)) {                               \
            case 4: _r = _mm_div_ps(a, b); break ;          \
            case 8: _r = _mm_div_pd(a, b); break ; } _r; })


    #if defined (__SSE4_1__)
        #include <smmintrin.h>
        #define extract_epi_64_imm _mm_extract_epi64
        #define extract_epi_32_imm _mm_extract_epi32

        #define vector_is_zeros(a) _mm_testz_si128(a, a)
        #define vector_are_eq(a, b) _mm_testz_si128(vector_xor(a, b), _mm_set1_epi32(-1))
    #else
        #define vector_is_zeros(a) (_mm_movemask_epi8(_mm_cmpeq_epi8(a, _mm_setzero_si128())) == 0xFFFF)
        #define vector_are_eq(a, b) (_mm_movemask_epi8(_mm_cmpeq_epi8(vector_xor(a, b), vector_zeros())) == 0xFFFF)

        #define _mm_mullo_epi32(a, b) ({                                            \
            vector_t _a_ = (a), _b_ = (b);                                          \
            _mm_unpacklo_epi32(                                                     \
                _mm_shuffle_epi32(_mm_mul_epu32(_a_, _b_), _MM_SHUFFLE (0,0,2,0)),  \
                _mm_shuffle_epi32(_mm_mul_epu32(_mm_srli_si128(_a_, 4), _mm_srli_si128(_b_, 4)), _MM_SHUFFLE (0,0,2,0))); })

        // sse2 can only extract the lower 64 bit number for the upper we need to either shuffer or shift
        #define extract_epi_64_imm(a, _im) ({                                         \
            typeof(_im) _m;                                                           \
            switch (_im) {                                                            \
                case 0: _m = _mm_cvtsi128_si64(a); break ;                                 \
                case 1: _m = _mm_cvtsi128_si64(_mm_srli_si128(a, 8));  break ;    \
            } _m; })

        // sse2 can only extract the lower 32 bits so we need to either shuffle or shift ...
        #define extract_epi_32_imm(_a, _imm) ({                                    \
            typeof(_imm) _m;                                                      \
            switch (_imm) {                                                       \
                case 0: _m = _mm_cvtsi128_si32(_a); break ;                     \
                case 1: _m = _mm_cvtsi128_si32(_mm_srli_si128(_a, 4)); break ;     \
                case 2: _m = _mm_cvtsi128_si32(_mm_srli_si128(_a, 8)); break ;     \
                case 3: _m = _mm_cvtsi128_si32(_mm_srli_si128(_a, 12)); break ;    \
            }                                                                     \
        _m; })
    #endif

    #define vector_entry_imm(a, imm_index) ({                        \
        typeof(imm_index) _val_;                                     \
        switch(sizeof(_val_)) {                                      \
            case 8: _val_ = extract_epi_64_imm(a, imm_index); break ; \
            case 4: _val_ = extract_epi_32_imm(a, imm_index); break ; \
            case 2: _val_ = _mm_extract_epi16(a, imm_index); break ; \
            case 1: _val_ = _mm_extract_epi8(a, imm_index);  break ; \
    } _val_; })


/*
    SSE doesn't support 64 bit multiplication, but it can be emulated assuming a & b vectors with 64 bit numbers
     assume a, b are 64 bit numbers and high/low return the corresponding 32 bits
     then a * b
        == (low(a) + high(a)) * (low(b) + high(b))
        == low(a)*low(b) + low(a)*high(b) + high(a)*low(b) + high(a)*high(b)
        == low(a)*low(b) + high(a)*high(b) + (high(a)*low(b) + high(a)*low(b))
        a * high(x) == a * ((x / 2**32) * 2**32) == (a * (x >> 32)) << 32

        vector_t high_a = _mm_shuffle_epi32(a, _MM_SHUFFLE(2, 3, 0, 1)), high_b = _mm_shuffle_epi32(a, _MM_SHUFFLE(2, 3, 0, 1));
        _mm_mul_epi32(a, b) multiplies the low parts of each 64 bit number.
        + _mm_mul_epi32(high_a, high_b) multiply high parts
        + _mm_mul_epi32(a,  high_b)
        + _mm_mul_epi32(high_a,  b)   // reqs2 shuffles, 4 mults, 3 adds (9 instrs)
 */
    #define vector_mul_i(a, b, i) ({\
        vector_t _r; \
        switch (sizeof(i)) {\
            case 2: _r = _mm_mullo_epi16(a, b); break ;\
            case 4: _r = _mm_mullo_epi32(a, b); break ; \
            case 8: {\
                vector_t _a = (a), _b = (b), \
                high_a = _mm_shuffle_epi32(_a, _MM_SHUFFLE(2, 3, 0, 1)), \
                high_b = _mm_shuffle_epi32(_b, _MM_SHUFFLE(2, 3, 0, 1)); \
                _r = vector_add_i(\
                    vector_add_i(_mm_mul_epu32(_a, _b),  _mm_mul_epu32(high_a, high_b), i), \
                    vector_add_i(_mm_mul_epu32(_a,  high_b), _mm_mul_epu32(high_a,  _b), i),\
                    i);}} _r;})

// SSE doesn't support byte shifts so just do word shifts since it will shift bytes anyway and with byte incase their all zero out, since words won't be ...
    #define vector_bits_lshft_imm(v, i) ({                                                  \
        vector_t _r;                                                                        \
        switch(sizeof(i)){                                                                  \
            case 1: _r = vector_and(                                                        \
                vector_broad_cast_i((char)(0xFFULL << (i))),                                \
                _mm_slli_epi32(v, i)                                                        \
            ); break ;                                                                      \
            case 2: _r = _mm_slli_epi16(v, i); break ;                                      \
            case 4: _r = _mm_slli_epi32(v, i); break ;                                      \
            case 8: _r = _mm_slli_epi64(v, i); break ;                                      \
        }                                                                                   \
    _r; })


// theres no right 64 bit arithmetic shift we emualted by first doing a logical left shift or it with 1s
// we load the duplicate the high 32 bits in each half and check if they are less than 0
// if they are they will be set to -1 if not they will be set to 0, then simply left shift the -1s
// zero(1, 0.3), shuffle(1, 0.5), lt(1, 0.5), sl(2), rsh(1, 1), or(1, 0.33) == 7 instrs
    #define vector_bits_arith_rshft_imm(v, i) ({            \
        vector_t _r;                                        \
        switch(sizeof(i)){                                  \
            case 1: _r = vector_and(vector_broad_cast_i((char)(-1 >> (i))), _mm_srai_epi32(v, i)); break ;   \
            case 2: _r = _mm_srai_epi16(v, i); break ;      \
            case 4: _r = _mm_srai_epi32(v, i); break ;      \
            case 8: {                                       \
                vector_t __t__ = (v);                       \
                _r = vector_or(                             \
                    vector_bits_logc_rshft_imm(__t__, i),   \
                    vector_bits_lshft_imm(                  \
                        _mm_cmplt_epi32(_mm_shuffle_epi32(__t__, _MM_SHUFFLE(3, 3, 1, 1)), _mm_setzero_si128()), \
                        (bit_size(i) - i)\
                    )); \
            } break ;} \
        _r; })

    #define vector_elems_logc_rshft(v, i) _mm_srli_si128(v, sizeof(i))

    #define vector_bits_logc_rshft_imm(v, i) ({         \
        vector_t _r;                                    \
        switch(sizeof(i)){                              \
            case 1: _r = vector_and(                    \
                vector_broad_cast_i((char)(0xFF >> (i))), _mm_srli_epi32(v, i)); break;    \
            case 2: _r = _mm_srli_epi16(v, i); break ;  \
            case 4: _r = _mm_srli_epi32(v, i); break ;  \
            case 8: _r = _mm_srli_epi64(v, i); break ;  \
        } \
    _r;})

    #define vector_broad_cast_i(a) ({                   \
        vector_t _temp;                                 \
        switch(sizeof(a)) {                             \
            case 1: _temp = _mm_set1_epi8(a); break ;   \
            case 2: _temp = _mm_set1_epi16(a); break ;  \
            case 4: _temp = _mm_set1_epi32(a); break ;  \
            case 8: _temp = _mm_set1_epi64x(a); break ;} _temp;})

    #define vector_add_i(a, b, t) ({                    \
        vector_t _r;                                    \
        switch (sizeof(t)) {                            \
            case 1: _r = _mm_add_epi8(a, b);  break ;   \
            case 2: _r = _mm_add_epi16(a, b); break ;   \
            case 4: _r = _mm_add_epi32(a, b); break ;   \
            case 8: _r = _mm_add_epi64(a, b); break ; } _r;})

    #define vector_sub_i(a, b, t) ({                    \
        vector_t _r;                                    \
        switch (sizeof(t)) {                            \
            case 1: _r = _mm_sub_epi8(a, b);  break ;   \
            case 2: _r = _mm_sub_epi16(a, b); break ;   \
            case 4: _r = _mm_sub_epi32(a, b); break ;   \
            case 8: _r = _mm_sub_epi64(a, b); break ; } _r;})




    #define vector_store_func(addr) ((((word_t)(addr)) % sizeof(vector_t)) ? &vector_store : &vector_store_algin)
    #define vector_zeros _mm_setzero_si128


#elif (UINTPTR_MAX == UINT64_MAX)
    #define vector_t            unsigned long long
    #define vector_load(a)      (*(vector_t *)(a))
    #define vector_load_align   vector_load
    #define vector_are_eq(a, b) ((a) == (b))
#else
    #define vector_t unsigned int

    inline vector_t vector_load(void *a){   return (*(vector_t *)(a));  }

    #define vector_load_align vector_load
    #define vector_are_eq(a, b) ((a) == (b))
#endif


#if defined (__SSE2__)
#define memset_1_byte(dest, pattern, cnt) \
    switch (((cnt) % sizeof(vector_t))) {\
        typeof(cnt) __index__; \
        vector_t vect_ptrn;   \
        void (*store_vect)(vector_t *, vector_t); \
        case 15: *(dest)++ = (pattern); case 14: *(dest)++ = (pattern);\
        case 13: *(dest)++ = (pattern); case 12: *(dest)++ = (pattern);\
        case 11: *(dest)++ = (pattern); case 10: *(dest)++ = (pattern);\
        case 9:  *(dest)++ = (pattern); case 8:  *(dest)++ = (pattern);\
        case 7:  *(dest)++ = (pattern); case 6:  *(dest)++ = (pattern);\
        case 5:  *(dest)++ = (pattern); case 4:  *(dest)++ = (pattern);\
        case 3:  *(dest)++ = (pattern); case 2:  *(dest)++ = (pattern);\
        case 1:  *(dest)++ = (pattern);\
        case 0:  vect_ptrn = _mm_set1_epi8(pattern); store_vect = vector_store_func((dest)); \
            for (__index__ = 0; __index__ < ((cnt) / sizeof(vector_t)); __index__++) \
                store_vect((vector_t *)(dest + (__index__ * sizeof(vector_t))), vect_ptrn);\
        (dest) += (__index__) * sizeof(vector_t);\
    }
#else

#define memset_1_byte(dest, pattern, cnt) \
    switch ((cnt) % sizeof(unsigned long long)) {\
        typeof(cnt) __index__; \
        case 7: *(dest)++ = (pattern); case 6: *(dest)++ = (pattern);\
        case 5: *(dest)++ = (pattern); case 4: *(dest)++ = (pattern);\
        case 3: *(dest)++ = (pattern); case 2: *(dest)++ = (pattern);\
        case 1: *(dest)++ = (pattern);\
        case 0: for (__index__ = 0; __index__ < ((cnt) / sizeof(unsigned long long)); __index__++) \
            ((unsigned long long *)(dest))[__index__] = broad_casts_1_8[(pattern)]; \
        (dest) += (__index__) * sizeof(unsigned long long);\
    }
#endif

#define vector_cnt(unit_s) (sizeof(vector_t)/sizeof(unit_s))


#endif