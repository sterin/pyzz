import pyzz

WORD_MASK = (1<<32)-1


def n_words(N):
    return 1 if N<=5 else 1 << (N -5)


def to_words(tt):
    return (int( (tt.d >> 32*i) & WORD_MASK) for i in xrange( n_words(tt.m.N)))


def from_words(m, words):
    tt = m.const(0)
    for i, w in enumerate(words):
        tt.d |= (w << i*32)
    return tt


def canonize(tt):
    mask, words, permutation = pyzz.abc_tt_canonize(tt.m.N, to_words(tt))
    return mask, from_words(tt.m, words), permutation 
