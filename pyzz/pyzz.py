import itertools

from _pyzz import *


SAT_VALUES = [ 'UNKNOWN', 'ERROR', 'UNSAT', 'SAT']
L_VALUES = [ 'l_Unknown', 'l_Error', 'l_False', 'l_True']

def all_signals(N):
    return itertools.chain( [N.get_True()], N.get_PIs(), N.get_Flops(), N.get_Ands(), N.get_POs() )

def all_fcs_for_fair_po(N, fair_po_no):
    return itertools.chain( N.get_fair_properties()[fair_po_no], N.get_fair_constraints() )

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

def seen_since(N, x, event):
    ff = N.add_Flop()
    next_ff = event.ite( x, ff|x )
    ff[0] = next_ff
    return next_ff, ff

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

def get_coi(N, roots, stop_at):
    
    dfs_stack = list(+w for w in roots)

    stop_at = set(stop_at)
    visited = set()
    
    while dfs_stack:
        
        w = dfs_stack.pop()
        
        if w in stop_at:
            continue

        if w in visited:
            continue
            
        visited.add(w)

        if w.is_Flop():
            dfs_stack.append(+w[0])
        if w.is_Buf():
            dfs_stack.append(+w[0])
        elif w.is_PO():
            dfs_stack.append(+w[0])
        elif w.is_And():
            dfs_stack.append(+w[0])
            dfs_stack.append(+w[1])

    return visited
    
def topological_order(roots, stop_at = []):

    visited = set(+w for w in stop_at)

    dfs_stack = [+w for w in roots if +w not in visited ]

    while dfs_stack:

        w = dfs_stack.pop()

        if w.sign():
            yield +w

        elif w not in visited:

            dfs_stack.append(~w)

            visited.add(w)

            if w.is_PO():
                dfs_stack.append(+w[0])
            elif w.is_And():
                dfs_stack.append(+w[0])
                dfs_stack.append(+w[1])
            elif w.is_Buf():
                dfs_stack.append(+w[0])

def copy_cone(N_src, N_dst, wires, stop_at={}):

    xlat = wwmap()

    xlat[N_src.get_True()] = N_dst.get_True()

    for k, v in stop_at.iteritems():
        xlat[k] = v

    flops = []
    flop_init = N_src.flop_init

    coi = get_coi(N_src, wires, stop_at)

    for ff in N_src.get_Flops():
        if ff in coi:
            xlat[ff] = N_dst.add_Flop(init=flop_init[ff])
            flops.append(ff)

    for w in topological_order(coi, stop_at):

        if w.is_And():
            xlat[w] = xlat[w[0]] & xlat[w[1]]

        elif w.is_PI():
            xlat[w] = N_dst.add_PI()

        elif w.is_PO():
            xlat[w] = N_dst.add_PO(fanin=xlat[w[0]])

        elif w.is_Buf():
            xlat[w] = xlat[w[0]]

    for w in flops:
        xlat[w][0] = xlat[w[0]]

    return xlat

def copy_coi(N, roots=None, M=None, stop_at={}):

    if M is None:
        M = netlist()

    if roots is None:
        roots = list(N.get_POs())

    xlat = copy_cone(N, M, roots, stop_at)

    return M, xlat

def combine_cones(*netlists):
    "create a new netlist and copy '*netlists' into it, mapping the PIs of all nelists to a single set of PIs"

    assert len(netlists)>0

    n_pis = max( M.n_PIs() for M in netlists )

    N = netlist()

    pis = [ N.add_PI() for _ in xrange(n_pis) ]

    xlats = [ copy_cone(M, N, [po[0] for po in M.get_POs()], stop_at=dict(zip(M.get_PIs(), pis))) for M in netlists ]

    return N, xlats
