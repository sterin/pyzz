import pyaig
from pyzz.tt import canonize

import pyzz

N = pyzz.netlist()

K = 10

prev = N.get_True()

for i in xrange(K):
    
    x = N.add_PI()
    prev = x & ~prev
    N.add_PO(fanin=prev)

N.write_aiger("large_or.aig")

if __name__=="__main__":

    n_vars = 6
    m = pyaig.truth_tables(n_vars)

    x0 = m.var(0, 1)
    x1 = m.var(1, 1)
    
    f = x0&x1

    mask, tt, perm = canonize(f)
    print bin(mask), tt, perm

    mask, tt, perm = canonize(~f)
    print bin(mask), tt, perm
    

    # for i in xrange(n_vars):
    #     for c in [0, 1]:
    #         v = m.var(i, c)
    #         res = canonize(m, v)
    #         print v, res[0], res[1], res[2]



