from _pyzz import *

import sys
import time
import random

from contextlib import contextmanager

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

class unrollx(object):
    
    def __init__(self, N, unroll=0, init = True):
        
        self.init = init
        
        self.N = N
        self.F = netlist()
        
        self.wmaps = [] 
        
        self.frames = 0

        self.unroll(unroll)
        
    def unroll_wire(self, w, k):
        
        if w.is_And():
            
            f = self.wmaps[k][w[0]] & self.wmaps[k][w[1]]
        
        elif w.is_PI():
            
            f = self.F.add_PI()

        elif w.is_Flop():
            
            if k == 0:
                if self.init:
                    f = ~self.F.get_True()
                else:
                    f = self.F.add_PI()
            else:
                f = self.wmaps[k-1][w[0]]

        elif w.is_PO():
            
            f = self.F.add_PO()
            f[0] = self.wmaps[k][w[0]]

        elif w.is_True():
            
            f = self.F.get_True()
        
        self.wmaps[k][w] = f 
        
    def unroll_frame(self):

        m = wwmap()
        m[ self.N.get_True() ] = self.F.get_True()
        
        self.wmaps.append( m )

        sinks = list(self.N.get_POs()) + list(self.N.get_Flops()) + list(self.N.get_PIs()) + list(self.N.get_Ands())
        
        for w in self.N.uporder( sinks ):
            self.unroll_wire(w, self.frames)
            
        self.frames += 1
        
    def unroll(self, k):
        
        for i in xrange(k):
            self.unroll_frame()
            
        return self

    def __getitem__(self, item):
        return self.map_one(item[0], item[1])
        
    def map(self, wires, k):
        
        wmap = self.wmaps[k]
        
        for w in wires:
            yield wmap[w]

    def map_one(self, w, k):
        
        return self.wmaps[k][w]

def ite(i, t, e):
    return i&t | ~i&e

def imply(lhs, rhs):
    return ~lhs | rhs

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

def conjunction(N, wires):

    def helper(wires):

        if len(wires)==0:
            return N.get_True()
    
        if len(wires)==1:
            return wires[0]
    
        return helper( wires[0:len(wires)/2]) & helper( wires[len(wires)/2:])
    
    return helper(list(wires))


def disjunction(N, wires):
    return ~conjunction(N, (~w for w in wires) )

def equal(N, wires1, wires2):
    return conjunction( N, ( ~( w1 ^ w2 ) for w1, w2 in zip(wires1, wires2) ) )
     
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

def get_coi(N, sinks):
    
    dfs_stack = list(+w for w in sinks)
    
    visited = set()
    
    while dfs_stack:
        
        w = dfs_stack.pop()
        
        if w in visited:
            continue
            
        visited.add(w)
        
        if w.is_Flop():
            dfs_stack.append(+w[0])
        elif w.is_PO():
            dfs_stack.append(+w[0])
        elif w.is_And():
            dfs_stack.append(+w[0])
            dfs_stack.append(+w[1])

    return visited
    
def copy_coi(N):
    
    coi = get_coi(N, N.get_POs())
    M = netlist()
    
    xlat = wwmap()
    xlat[N.get_True()] = M.get_True()
    
    for w in N.uporder(coi):
        
        if w.is_PI():
            xlat[w] = M.add_PI()
        
        elif w.is_Flop():
            ff = M.add_Flop()
            xlat[w] = ff

        elif w.is_And():
            xlat[w] = xlat[w[0]]&xlat[w[1]]
        
        elif w.is_PO():
            po = M.add_PO()
            po[0] = xlat[w[0]]
            xlat[w] = po

    for w in N.get_Flops():
        if w in coi:
            xlat[w][0] = xlat[w[0]]

    return M, xlat

def chain( *iters ):
    for it in iters:
        for e in it:
            yield e

def all_signals(N):
    return chain( [N.get_True()], N.get_PIs(), N.get_Flops(), N.get_Ands(), N.get_POs() )

def arg_netlist(default):
    
    import sys

    if len(sys.argv) < 2:
        filename = default
    else:
        filename = sys.argv[1]
    
    N = netlist.read_aiger(filename)
    N, _ = copy_coi(N)
    
    return N

def simulate_assignment(S, N, signals):
    
    s_vals = [ None, None, 0, 1]
    
    def s_val(s):
        if s == solver.l_Undef:
            return random.randint(0,0xFFFFFFFF)
        return s_vals[s]
    
    def sv(w):
        return s_val(S.value(w))*0xFFFFFFFF if w in S else random.randint(0,0xFFFFFFFF)
        
    res = { N.get_True() : 0xFFFFFFFF }
    
    def v(w):
        return res[+w]^(w.sign()*0xFFFFFFFF)
    
    for w in N.uporder( signals ):

        if w.is_PI():
            res[w] = sv(w)
        elif w.is_Flop():
            res[w] = sv(w)
        elif w.is_PO():
            res[w] = v(w[0])
        elif w.is_And():
            res[w] = v(w[0])&v(w[1])
    
    return res

def somepast(N, x):
    ff = N.add_Flop()
    next_ff = ff | x
    ff[0] = next_ff
    return next_ff, ff
    
def past_since( N, x, trigger):
    return somepast( N, x&somepast( N, trigger) )

def monotone(N):
    pi = N.add_PI()
    ff = N.add_Flop()
    monotone = ff | pi
    ff[0] = monotone
    monotone_start = ~ff & monotone
    return monotone, monotone_start

def sample(N, s, x):
    ff = N.add_Flop()
    next_ff = s.ite( x, ff )
    ff[0] = next_ff
    return next_ff
    
def rigid(N, init):
    pi = N.add_PI()
    ff = N.add_Flop()
    next_ff = init.ite( pi, ff )
    ff[0] = next_ff
    return next_ff
    
def filter_underscore(x):
    return not x.startswith("_")
    
def filter_all(x):
    return True
    
def filter_none(x):
    return False

def print_cex( U, S, symbols, loop=None, filter=filter_underscore ):

    v = ["?", "!", "0", "1"]
    
    filtered_symbols = [ sym for sym in symbols.keys() if filter(sym) ]
    
    if not filtered_symbols:
        return
    
    maxlen = +max( len(sym) for sym in filtered_symbols )
    
    res = simulate_assignment(S, U.F, all_signals(U.F))
    
    if "_LIVENESS_LOOP_START" in symbols:
        lls = symbols["_LIVENESS_LOOP_START"]
        for i in xrange(U.frames):
            if S[U[lls,i]] == solver.l_True:
                loop=i+1
    
    for sym in sorted(filtered_symbols):
        
        w = symbols[sym]
        
        print "%-*s:"%(maxlen,sym),

        for i in xrange(U.frames):
            if loop is not None and loop==i :
                print "|",
            x = U[w,i]
            v = (res[+x]&1)^x.sign() if +x in res else 'X'
            print v,

        print
        
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
