

#if !defined(__SEQ__)
#define __SEQ__
#include "bits.h"

typedef struct seq_t {
    word_t *base;
    unsigned long cnt;
} seq_t;
#define seq_base(seq)             (*(typeof(seq->base) **)(seq))
#define seq_word_size             sizeof(((seq_t){}.base[0]))
#define seq_word_bit_size         bit_size(((seq_t){}.base[0]))

intrsc_signat(word_t) seq_get(seq_t *seq, unsigned long index, unsigned bit_mag);

#define seq_set_start_p(seq, p)     (seq_strt_p(seq) = (p))
#define seq_end_p(seq)              ((seq)->end_p)
#define seq_set_end_p(seq, p)       (seq_end_p(seq) = (p))
#define seq_byte_mag(seq)           (seq_end_p(seq) - seq_strt_p(seq))
#define seq_bit_mag(seq)            (seq_byte_mag(seq) * BYTE_BIT_SIZE)
#define seq_is_empty(seq)           (!seq_byte_mag(seq))

unsigned long seq_shrd_cnt(seq_t *seq_a, seq_t *seq_b);

#endif

