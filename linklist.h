
#if !defined(__LINKLIST__)
#define __LINKLIST__

typedef struct ll_t {struct ll_t *_next; unsigned value;} ll_t;

#define ll_next(stck) (*((ll_t **)(stck)))
#define ll_set_next(ll, n) (ll_next(ll) =  (n))
#define ll_value(ll) ((ll)->value)
#define ll_set_value(ll, v) (ll_value(ll) = (v))

#define ll_blk_alloc(blck) (blck++)
#define ll_init(ll, val, nxt) (ll_set_value(ll, val), ll_set_next(ll, nxt))

#endif