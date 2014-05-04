import itertools

from ._pyzz import *

def all_signals(N):
    return itertools.chain( [N.get_True()], N.get_PIs(), N.get_Flops(), N.get_Ands(), N.get_POs() )

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
    return conjunction( N, ( w1.equals(w2) for w1, w2 in zip(wires1, wires2) ) )
     
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

def rigid(N):
    ff = N.add_Flop(init=solver.l_Undef)
    ff[0] = ff
    return ff

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
    
def copy_coi(N, roots=None):

    if not roots:
        roots = N.get_POs()

    coi = get_coi(N, roots)

    M = netlist()

    xlat = wwmap()
    xlat[N.get_True()] = M.get_True()

    flops = []

    for w in N.uporder(coi):

        if w.is_PI():
            xlat[w] = M.add_PI()

        elif w.is_Flop():
            flops.append(w)
            xlat[w] = M.add_Flop(init=N.flop_init[w])

        elif w.is_And():
            xlat[w] = xlat[w[0]]&xlat[w[1]]

        elif w.is_PO():
            xlat[w] = M.add_PO(fanin=xlat[w[0]])

    for w in flops:
        xlat[w][0] = xlat[w[0]]

    return M, xlat
