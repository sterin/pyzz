import sys
import time
import random

from contextlib import contextmanager

from .pyzz import *

class fmap(object):
    
    def __init__(self, seq=None):
        self.m = dict()
        
        if seq:
            for k,v in seq:
                self[k] = v
                
    def __getitem__(self, k):
        return self.m[+k]^k.sign()
        
    def __setitem__(self, k, v):
        self.m[+k] = v^k.sign()
        
    def __contains__(self, k):
        return +k in self.m
        
    def __delitem__(self, k):
        del self[+k]

class wset(object):

    def __init__(self, seq=None):
        self.m = set()

        if seq:
            for k,v in seq:
                self[+k] = v

    def add(self, w):
        self.m.add(+w)

    def remove(self, w):
        self.m.remove(+w)

    def __contains__(self, k):
        return +k in self.m

def cyclic(S):
    
    iter = S.__iter__()
    first = next(iter)
    
    yield first
    
    for s in iter:
        yield s
        
    yield first
    
def pairs(S):
    
    iter = S.__iter__()
    
    prev = next(iter)
    
    for s in iter:
        yield prev, s
        prev = s
    
def cyclic_pairs(S):
    return pairs(cyclic(S))

def make_symbols(N):

    symbols = { "PI_%d"%i:pi for i,pi in enumerate(N.get_PIs())}

    symbols.update( {"Flop_%d"%i:ff for i,ff in enumerate(N.get_Flops()) })
    symbols.update( {"P_%d"%i:po[0]^po.sign() for i,po in enumerate(N.get_properties()) } )
    symbols.update( {"C_%d"%i:po[0]^po.sign() for i,po in enumerate(N.get_constraints()) } )

    for i, fair_prop in enumerate(N.get_fair_properties()):
        symbols.update( {"FP_%d_%d"%(i,j):po[0]^po.sign() for j,po in enumerate(fair_prop) } )

    symbols.update( {"FC_%d"%i:fc[0]^fc.sign() for i,fc in enumerate(N.get_fair_constraints()) } )

    return symbols

def arg_netlist(default):

    import sys

    if len(sys.argv) < 2:
        filename = default
    else:
        filename = sys.argv[1]

    N = netlist.read_aiger(filename)
    N.remove_unreach()

    return N, make_symbols(N)
        
class verification_instance(object):

    def __init__(self, N=None, symbols=None, init_constraints=None, constraints=None, pos=None, fairnesses=None, init = None):

        self.N = N or netlist()
        self.symbols = symbols or {}
        self.init = init
        self.init_constraints = init_constraints or []
        self.constraints = constraints or []
        self.pos = pos or []
        self.fairnesses = fairnesses or []
        
    def get_init(self):
        
        if not self.init:
            
            ff = self.N.add_Flop()
            ff[0] = self.N.get_True()
            self.init = ~ff
            
        return self.init

class cex(object):
    
    def __init__(self, U, S):
        
        self.flops = [ S.value(uff) for uff in U.map( U.N.get_Flops(), 0) ]
        self.pis = []
        self.violation = -1
        
        for f in xrange(U.frames):
            self.pis.append( [ S.value(upi) if upi else solver.l_Undef for upi in U.map( U.N.get_PIs(), f) ] )
            
        for f in xrange(U.frames):
            for upo in U.map( U.N.get_POs(), f):
                if S.value( upo ) == solver.l_True:
                    self.violation = f
                    return
            
    def __str__(self):
        
        res = []
        V = { solver.l_True:"1", solver.l_False:"0", solver.l_Undef:'?', solver.l_Error:"!" }

        res.append( "FFs : " )
        res.append( "".join(V[ff] for ff in self.flops) )
        res.append( "\n" )
        
        for f in xrange(self.violation+1):
            res.append( "%4d: "%f )
            res.append( "".join(V[pi] for pi in self.pis[f]) )
            res.append( "\n" )

        return "".join(res)


@contextmanager
def measure(name):
    start = time.time()
    yield
    end = time.time()
    print "%s: %.2fs"%(name, end-start)

def i_lt_j(n):
    for j in xrange(n):
        for i in xrange(j):
            yield (i, j)

def pyzz_to_pyaig(N):

    from pyaig import AIG, write_aiger

    aig = AIG()
    xlat = fmap()

    xlat[N.get_True()] = AIG.get_const1()

    for pi in N.get_PIs():
        xlat[pi] = aig.create_pi()

    flop_init = N.flop_init
    lbool_to_init = [AIG.INIT_NONDET, AIG.INIT_NONDET, AIG.INIT_ZERO, AIG.INIT_ONE]

    for ff in N.get_Flops():
        xlat[ff] = aig.create_latch(init=lbool_to_init[flop_init[ff]])

    if N.n_Ands()>0:
        for w in N.uporder(N.get_Ands()):
            if w.is_And():
                xlat[w] = aig.create_and( xlat[w[0]], xlat[w[1]])

    for ff in N.get_Flops():
        aig.set_next(xlat[ff], xlat[ff[0]])

    def xlat_po(po):
        return xlat[ po[0]^po.sign() ]

    for p in N.get_properties():
        aig.create_po( xlat_po(~p), po_type=AIG.BAD_STATES)

    for c in N.get_constraints():
        aig.create_po(xlat_po(c), po_type=AIG.CONSTRAINT)

    for fp in N.get_fair_properties():
        j_pos = [ aig.create_po(xlat_po(f), po_type=AIG.JUSTICE) for f in fp ]
        aig.create_justice(j_pos)

    for fc in N.get_fair_constraints():
        aig.create_po(xlat_po(fc), po_type=AIG.FAIRNESS)

    print [+f for fp in N.get_fair_properties() for f in fp]

    all_prop_pos = set( itertools.chain(
        (+p for p in N.get_properties()),
        (+c for c in N.get_constraints()),
        (+fc for fc in N.get_fair_constraints()),
        (+f for fp in N.get_fair_properties() for f in fp)
    ))

    for po in N.get_POs():
        if po not in all_prop_pos:
            aig.create_po(xlat_po(po), po_type=AIG.OUTPUT)

    return aig, xlat
