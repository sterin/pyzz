import time
from pyzz import *

def zzz():
    
    N = netlist()
    
    X1 = N.add_PI()
    X2 = N.add_PI()
    
    PO1 = N.add_PO()
    PO2 = N.add_PO()
    
    PO1[0] = X1
    PO2[0] = X2
    
    N.write_aiger("xxx.aig")
    
zzz()
import sys
sys.exit(1)

def lfsr():

    N = netlist()
    
    K = 4

    enable = N.add_PI()
    ffs = [ N.add_Flop() for _ in xrange(K) ]

    ffs[0][0] = ite(enable, ~ffs[1], ffs[0])
    ffs[0] = ~ffs[0]

    for i in xrange(1, K-1):
        ffs[i][0] = ite(enable, ffs[i+1], ffs[i])

    ffs[-1][0] = ite(enable, ffs[3]^ffs[0], ffs[-1] )

    po = N.add_PO()
        
    po[0] = ~ffs[0] & ffs[1] & ~ffs[2] & ~ffs[3]
    
    return N

N = lfsr()
N.write_aiger("lfsr.aig")

def xxx():
    N = netlist()
    
    en = N.add_PI(10)
    ff = N.add_Flop(20)

    ff[0] = ite(en, ~ff, ff)
    
    po = N.add_PO()
    po[0] = ff
    
    return N
    

def bmc(unroll, N, max):
    
    U = unroll(N)
    S = solver(U.F)

    proved_pos = []
    
    total_start = time.time()

    for frame in xrange(max):
    
        iteration_start = time.time()
    
        try:

            unroll_start = time.time()
            U.unroll(1)
            unroll_end = time.time()

            for j, po in enumerate(U.map( N.get_POs(), frame)):
                
                solve_start = time.time()
                rc = S.solve(proved_pos + [po])
                #~ rc = S.solve([po])
                solve_end = time.time()

                if rc == l_True:
                    return True, cex(U, S)
                    
                elif rc == l_False:
                    #~ print "%3d: UNSAT"%frame
                    proved_pos.append( ~po )
                else:
                    return False, None

        finally:
            iteration_end = time.time()
            print "iteration %5d:    unroll=%8.2fs,    solve=%8.2fs,    iteration=%8.2fs,    total=%8.2fs"%( frame, unroll_end-unroll_start, solve_end-solve_start, iteration_end-iteration_start, iteration_end - total_start )

    return False, None


def eqclasses(object):
    
    def __init__(self, N):
        pass
        
    def cex(self, S, k, base=True):
        pass
        
    def assumption(self, U, k):
        pass
        
    def assertion(self, U, k):
        pass

def induction( prop, K):
    
    base = unroll(prop.N, True)
    step = unroll(prop.N, False).unroll(1)

    base_solver = solver(base.F)
    step_solver = solver(step.F)
    
    base_assumptions = []
    step_assumptions = []
    
    for k in xrange(K):
        
        base.unroll(1)
        base_assumptions = prop.assumption( base, k )
        base_assertion = prop.assertion( base, k)
        
        rc = base_solver.solve( base_assumptions + [base_assertion] )
        
        
        
        
        step.unroll_frame()
        
        step_assumptions = prop.assume( step, k )
    
        base_

N = netlist.read_aiger("intel038.aig")
print bmc(unroll, N, 100)

