

#ifndef __ALLOC__
#define __ALLOC__

#include <stdlib.h>
#include <unistd.h>

#include "bits.h"


// the following assumes an address layout as: (0) static || heap() -> | <- stack() (2**n), where || is fixed and | can move either way...
#define alloc_is_static(ptr) ((void *)(ptr) < sbrk(0))  // static allocation ie (global), assuming malloc doesn't use sbrk
#define alloc_is_local(ptr) ((void *)&((char){0}) < (void *)(ptr)) // stack allocation
#define alloc_is_dyn(ptr)  (!(alloc_is_static(ptr) || alloc_is_local(ptr))) // dynamic allocation ie non-static and non-global



#define alloc_cnt_t unsigned long

#define type_frm_prefx(id) id ## _t
#define type_recl_ll(id) id ## _recld

#define recl_next(obj) ((obj)->_next)
#define recl_set_next(obj, v) (recl_next(obj) = (v))
#define recl_rem(recl_objs) ({typeof(recl_objs) _recl_obj = recl_objs; recl_objs = recl_next(_recl_obj); _recl_obj;})
#define recl_add(recl_objs, obj) (recl_set_next(obj, recl_objs), (recl_objs = obj))
#define recl_not_empty(id) is_not_null(type_recl_ll(id))

#define alloc_block_cnt(block) ((block)->_cnt)
#define alloc_block_empty(block) (!alloc_block_cnt(block))
#define alloc_block_dec_cnt(block) (--alloc_block_cnt(block))
#define alloc_block_values(block) ((block)->_block)
#define alloc_block_rem(block) (&alloc_block_values(block)[alloc_block_dec_cnt(block)])


#define recl_tmpl(id) type_frm_prefx(id) \
    *type_recl_ll(id) = NULL; \
    void id ## _recl(type_frm_prefx(id) *obj) {if (__builtin_expect(is_null(obj), 0)) return ; recl_add(type_recl_ll(id), obj); }

#define alloc_templ(id) \
    type_frm_prefx(id) *id ## _alloc() {\
        typedef struct alloc_block_t {type_frm_prefx(id) *_block; alloc_cnt_t _cnt;} alloc_block_t; \
        static alloc_block_t alloc_block;\
        if (recl_not_empty(id)) return recl_rem(type_recl_ll(id));\
        if (!alloc_block._cnt) alloc_block = (alloc_block_t){._block = malloc(getpagesize()), ._cnt = (getpagesize() / sizeof(type_frm_prefx(id)))};\
        return (__builtin_expect(alloc_block._block != NULL, 1) ? &alloc_block._block[--alloc_block._cnt] : NULL);\
    }



#define alloc_rec_templs(id) recl_tmpl(id) alloc_templ(id)

#endif