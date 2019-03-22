from pyzz import *

class dual(object):
    
    def __init__(self, H, L):
        self.H = H
        self.L = L
        
    @staticmethod
    def bool(b):
        return dual(b, not b)
        
    @staticmethod
    def T():
        return dual(True, False)
    
    @staticmethod
    def F():
        return dual(False, True)
        
    @staticmethod
    def X():
        return dual(True, True)
        
    @staticmethod
    def E():
        return dual(False, False)

    def __and__(self, rhs):
        if not self.L and not self.H:
            return dual.E()
        if not rhs.L and not rhs.H:
            return dual.E()

        return dual( self.H&rhs.H , self.L|rhs.L )
    
    def __or__(self, rhs):
        if not self.L and not self.H:
            return dual.E()
        if not rhs.L and not rhs.H:
            return dual.E()

        return dual( self.H|rhs.H , self.L&rhs.L )
    
    def __xor__(self, rhs):

        if type(rhs)==bool:
            rhs = dual.bool(rhs)

        if not self.L and not self.H:
            return dual.E()
        if not rhs.L and not rhs.H:
            return dual.E()

        if self.L and self.H:
            return dual.X()
        if rhs.L and rhs.H:
            return dual.X()

        return dual( self.H^rhs.H , not ( self.L^rhs.L ) )

    def __neg__(self):
        return dual(self.L, self.H)

    def __eq__(self, rhs):
        return self.L == rhs.L and self.H == rhs.H

    def __repr__(self):

        if self.H and self.L:
            return "X"
        if self.H and not self.L:
            return "T"
        if not self.H and self.L:
            return "F"
        else:
            return "E"


def get_bcoi(N):
    return set( N.uporder( N.get_POs() ) )
    
def not_equal( N, ws1, ws2, bcoi ):
    
    res = ~N.get_True()

    for w1, w2 in zip(ws1, ws2):
        if +w1 in bcoi and +w2 in bcoi:
            res |= w1^w2
        
    return res

def not_equal_states(U, i, j, bcoi):
    return not_equal( U.F, U[U.N.get_Flops(), i], U[U.N.get_Flops(), j], bcoi)

def unique(U):
    
    bcoi = get_bcoi(U.F)
    res = U.F.get_True()
    
    for i, j in i_lt_j(U.frames):
        res &= not_equal_states(U,i,j, bcoi)
    
    return res
    
def assert_unique(U, S):
    
    bcoi = get_bcoi(U.F)
    all_flops = []
    
    for i in xrange(U.frames):
        all_flops += list( U[U.N.get_Flops(),i] )
    
    values = simulate(S, U.F) #, sinks=all_flops)
    
    def v(w):
        return values[+w]^w.sign()
    
    for i, po in enumerate( U.F.get_POs() ):
        print "PO %d"%i, v(+po)
    
    for i,j in i_lt_j(U.frames):
        
        V_i = [ v(w) for w in U[U.N.get_Flops(), i] ]
        V_j = [ v(w) for w in U[U.N.get_Flops(), j] ]
        
        print "".join(str(v) for v in V_i)
        print "".join(str(v) for v in V_j)
        print
        
        assert V_i != V_j, "i=%s, j=%s"%(i,j)
        
def simulate(S, N, sinks=None):
    
    res = { N.get_True() : dual.T() }
    
    def v(w):
        return res[+w]^w.sign()
    
    s_to_d_vals = [ dual.X(), dual.E(), dual.F(), dual.T() ]
    
    def s_to_d(s):
        return s_to_d_vals[s]
    
    def sv(w):
        if w in S:
            return s_to_d( S.value(w) )
        return dual.X()
        
    if not sinks:
        sinks = list(N.get_POs()) + list(N.get_Flops()) + list(N.get_PIs())
    
    for w in N.uporder( sinks ):
    
        if w.is_PI():
            res[w] = sv(w)
        elif w.is_Flop():
            res[w] = sv(w)
        elif w.is_PO():
            res[w] = v(w[0])
        elif w.is_And():
            res[w] = v(w[0])&v(w[1])
        
    return res

def coi(N, sinks):
    
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

def step(N, k):

    po = disjunction( N, N.get_POs() )
 
    U = unroll(N, unroll=k+1, init=False)
    S = solver(U.F)

    for i in xrange(k):
        S.clause( [ ~U[po, i] ] )

    S.clause( [ U[po, k] ] )
    S.clause( [ unique(U) ] )
    
    rc = S.solve([])
    
    if rc == solver.l_True:
        assert_unique(U, S)
        print "SAT"
        
    elif rc == solver.l_False:

        print "UNSAT"
        return rc

    return solver.l_Undef
    
import sys

if len(sys.argv) < 2:
    filename = "aiger/intel004.aig"
else:
    filename = sys.argv[1]


N = netlist.read_aiger(filename)

print step(N, 20)
