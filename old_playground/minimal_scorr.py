from pyzz import *
import random

def initial_classes(N, signals):

    signals = list(signals)
    
    S = solver(N)
    S.clause( N.get_POs() )

    assert S.solve() == solver.l_True

    print "IC"

    res = simulate_assignment(S, N, signals)

    return [ [ w^res[w] for w in signals ] ]

def split( classes, U, S ):
    
    sim = simulate_assignment(S, U.F, all_signals(U.F) )
    
    def nsim(w,k):
        x = U[w,k]
        return sim[+x]^x.sign()
    
    def v(w):
        res = tuple( nsim(w,k) for k in xrange(U.frames) )
        return res

    res = []
    
    for origclass in classes:
        
        split_classes = {}
        
        for w in origclass:
            split_classes.setdefault( v(w), [] ).append( w )

        for newc in split_classes.values():
            if len(newc)>1:
                res.append(newc)

    return res
    

class assignment( object ):
    
    def __init__(self, U, S):
        self.U = U
        self.S = S
        self.sim = simulate_assignment(S, U.F, all_signals(U.F))
        self.cache = {}

    def _nsim(self, w,k):
        x = self.U[w,k]
        return self.sim[+x]^x.sign()

    def v(self, w):
        if w in self.cache:
            return self.cache[w]
        res = tuple( self._nsim(w,k) for k in xrange(self.U.frames) )
        self.cache[w] = res
        return res
        

def split_2( wires, start, uf, reprs, U, S ):
    
    assgn = assignment(U, S) 
    
    new_reprs = {}

    def get_repr(w):
        
        r = reprs[w]
        
        vr = assgn.v(r)
        vw = assgn.v(w)
        
        if vw == vr:
            return r

        R  = new_reprs.setdefault( r, {} )
        return R.setdefault( vw, w )

    for w in wires[start:]:
                
        r = reprs[w]
        
        if uf.same(r,w):
            continue
        
        reprs[w] = get_repr(w)

def get_classes_2( wires, uf ):
    classes = {}
    for w in wires:
        classes.setdefault( uf.find(w), [] ).append(w)
    return classes

def print_classes( prefix, x, classes):
    print prefix,
    for eqclass in classes:
        if x in eqclass or ~x in eqclass:
            print "%d*"%len(eqclass),
        else:
            print len(eqclass),
    print

def get_reprs(classes):
    
    reprs = {}

    for eqclass in classes:
        r = eqclass[0]
        for w in eqclass:
            reprs[w] = r
    
    return reprs
    
def al_diff(U, S, k, r, w):
    activation_literals = []
    for i in xrange(k):
        al = S.new_var()
        S.clause( [~U[r,i],U[w,i]], control=al)
        S.clause( [U[r,i],~U[w,i]], control=al)
        activation_literals.append(al)
    al = S.new_var()
    S.clause( activation_literals, control=al)
    return al            

def base_2(N, classes, k):

    U = unroll(N, unroll=k, init=False)
    S = solver(U.F)
    
    for i in xrange(k):
        S.clause( U[N.get_POs(),i] )
    
    reprs = get_reprs(classes)
    wires = sorted( chain(classes), key=lambda w: w.id() )
    
    uf = union_find(wires)
    
    for i, w in enumerate(wires):
        
        r = reprs[w]
        
        if uf.same(r,w):
            continue
            
        al = al_diff(U, S, k, r, w)
        
        res = S.solve(al)
        
        if res == solver.l_True:
            print "SAT"
            split_2(wires, i, uf, reprs, U, S)
        
        if res == solver.l_False:
            print "UNSAT"
            uf.union(r,w)
            
    return get_classes_2(wires, uf)
        
def base(N, classes, k):
    

    U = unroll(N, unroll=k, init=False)
    S = solver(U.F)

    while classes:
        
        print_classes("B(%d):"%k, N.get_True(), classes)
        
        activation_literals = []
        
        for eqclass in classes:

            frame_literals = []

            for i in xrange(k):
                al = S.new_var()
                S.clause( ( U[w,i] for w in eqclass), control = al)
                S.clause( (~U[w,i] for w in eqclass), control = al)
                frame_literals.append(al)
                
            al = S.new_var()
            
            S.clause( frame_literals, control=al)
            
            activation_literals.append(al)
            
        ax = S.new_var()
        S.clause( activation_literals , control = ax )
        
        rc = S.solve( U[bad, k-1] , ax )

        if rc == solver.l_False:
            return classes

        classes = split(classes, U, S)

        S.clause([~al])
        
        for al in activation_literals:
            S.clause([~al])


    return classes

def constr_from_classes( U, classes, k ):
    
    return conjunction( U.F, ( imply(a,b) for a,b in chain( cyclic_pairs ( U[eq, k] ) for eq in classes ) ) )

def step(N, classes, k):
    
    U = unroll(N, unroll=2, init=False)
    S = solver(U.F)
    
    while classes:
        
        print_classes("S:", N.get_True(), classes)
        
        constr = [constr_from_classes(U, classes, i) for i in xrange(k+1)]
        constr[0] = ~constr[0]
        
        rc = S.solve( *constr )
        
        if rc == solver.l_False:
            return classes
            
        classes = split(classes, U, S) 

    return classes


def constraints(N, signals):
    
    classes = initial_classes(N, signals)

    for i in xrange(1,10):
        classes = base_2(N, classes, i)

    classes = step(N, classes, 1)
    
    return classes
    
N = arg_netlist("aiger/intel002.aig")

classes = constraints(N, all_signals(N))

for eqclass in classes:
    print eqclass


