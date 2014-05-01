from pyzz import *
   
def not_equal( N, ws1, ws2):
    
    res = ~N.get_True()

    for w1, w2 in zip(ws1, ws2):
        res |= w1^w2
        
    return res

def not_equal_states(U, i, j):
    return not_equal( U.F, U[U.N.get_Flops(), i], U[U.N.get_Flops(), j])

def unique(U):
    
    res = U.F.get_True()
    
    for i, j in i_lt_j(U.frames):
        res &= not_equal_states(U,i,j)
    
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
M, xlat = copy_coi(N)

print len(list(M.get_Flops()))

print step(M, 18)
