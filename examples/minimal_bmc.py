from pyzz import *

def bmc(N, max, symbols, filter=lambda sym: True, cex=True):

    U = unroll(N, init=True)
    S = solver(U.F)
    
    po = disjunction( N, N.get_POs() )
 
    for frame in xrange(max):
        
        print "frame %5d: "%frame,
        
        U.unroll(1)
    
        fpo = U[po, frame]
    
        rc = S.solve( fpo )
        
        if rc == solver.l_True:
            
            print "SAT\n"
            if cex:
                print_cex(U, S, symbols, filter=filter)
            
            return rc
            
        elif rc == solver.l_False:

            print "UNSAT"
            S.clause( [ ~fpo ] )

    return solver.l_Undef

if __name__ == "__main__":
    print bmc( arg_netlist("aiger/intel038.aig") , 40, {})