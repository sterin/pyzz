from pyzz import *

def not_equal( N, ws1, ws2 ):
    return disjunction( N, ( w1 ^ w2 for w1, w2 in zip(ws1, ws2) ) )

def not_equal_states(U, i, j):
    return not_equal( U.F, U[U.N.get_Flops(), i], U[U.N.get_Flops(), j] )

def unique(U):
    return conjunction( U.F, ( not_equal_states(U, i, j) for i,j in i_lt_j(U.frames) ) )

def induction(N, max):

    po = disjunction( N, N.get_POs() )
 
    UB = unroll(N, init=True)
    SB = solver(UB.F)
    
    US = unroll(N, unroll=1, init=False)
    SS = solver(US.F)

    SS.clause( [ US[~po,0] ] )
    
    for frame in xrange(max):
        
        print "Base %5d: "%frame,
        
        UB.unroll(1)
    
        fbpo = UB[po, frame]
    
        rc = SB.solve( [ fbpo ] )
        
        if rc == solver.l_True:
            print "SAT"
            return rc
            
        elif rc == solver.l_False:
            print "UNSAT"
            SB.clause( [ ~fbpo ] )

        print "Step %5d: "%frame,
        
        US.unroll(1)
        
        fspo = US[ po, frame+1 ]
    
        rc = SS.solve( [ fspo, unique(US) ] )
        
        if rc == solver.l_True:
            SS.clause( [~fspo] )
            print "SAT"
            
        elif rc == solver.l_False:
            print "UNSAT"
            return rc

    return solver.l_Undef
    

print induction( arg_netlist() , 50)
