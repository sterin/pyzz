from pyzz import *

def cnf_interpolation(N, k):

    flops = list(N.get_Flops())
    
    Ubmc = unroll(N, unroll=k, init=False)
    Sbmc = solver(U.F)
    
    R = conjunction( ~w for w flops )
    
    while True:

            
        
    

N = arg_netlist("aiger/intel001.aig")
cnf_interolation(N)