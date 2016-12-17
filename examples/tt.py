import pyaig
from pyzz.tt import canonize


if __name__=="__main__":

    n_vars = 6
    m = pyaig.truth_tables(n_vars)

    for i in xrange(n_vars):
        v = m.var(i, 1)
        res = canonize(m, v)
        print v, res[0], res[1], res[2]
