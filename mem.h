
#if !defined(__UTILSMEM__)
#define __UTILSMEM__

#include "bits.h"

#define _remove_x(x)
#define conv_to_macro_func(x) _remove_x x
#define cons_paren(x) conv_to_macro_func x // convs ((expr_a)expr_b) to expr_b by creating conv_to_macro_func((expr_a)expr_b) -> _remove_x(expr_a)expr_b -> expr_b

#if cons_paren(NULL) // if NULL is non-zero use comparison operators
    #define is_not_null(v)  (NULL != (v))
    #define is_null(v)      (NULL == (v))
#else   // NULL must be zero so no need for comparison ..
    #define is_not_null(v)  (v)
    #define is_null(v)      (!is_not_null(v))
#endif

#define array_cnt(arr) (sizeof(arr)/sizeof((arr)[0]))

extern unsigned long long int broad_casts_1_8[256];



#endif
