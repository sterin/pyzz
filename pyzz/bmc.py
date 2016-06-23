import random

from .pyzz import unroll, solver, disjunction, all_signals
from .utils import make_symbols

def filter_underscore(x):
    return not x.startswith("_")

def filter_all(x):
    return True

def filter_none(x):
    return False

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
        elif w.is_Buf():
            res[w] = v(w[0])
        elif w.is_PO():
            res[w] = v(w[0])
        elif w.is_And():
            res[w] = v(w[0])&v(w[1])

    return res

def print_cex( U, S, symbols, loop=None, filter=filter_underscore ):

    v = ["?", "!", "0", "1"]

    filtered_symbols = [ sym for sym in symbols.keys() if filter(sym) ]

    if not filtered_symbols:
        return

    maxlen = +max( len(sym) for sym in filtered_symbols )

    res = simulate_assignment(S, U.F, all_signals(U.F))

    if "_LIVENESS_LOOP_START" in symbols:
        lls = symbols["_LIVENESS_LOOP_START"]
        for i in xrange(len(U)):
            if S[U[lls,i]] == solver.l_True:
                loop=i+1
                break

    for sym in sorted(filtered_symbols):

        w = symbols[sym]

        print "%-*s:"%(maxlen,sym),

        for i in xrange(len(U)):
            if loop is not None and loop==i :
                print "|",
            x = U[w,i]
            v = (res[+x]&1)^x.sign() if +x in res else 'X'
            print v,

        print

def simple_safety_bmc(N, bad, constr, max, start_frame, handle_sat, handle_unsat):

    bad = disjunction(N, bad)

    U = unroll(N, init=True)
    S = solver(U.F)

    for frame in xrange(max):

        start_frame(U, S, frame)

        fbad = U[bad, frame]
        S.cube( U[constr, frame] )

        rc = S.solve( fbad )

        if rc == solver.SAT:

            handle_sat(U, S, frame)
            return rc

        elif rc == solver.UNSAT:

            handle_unsat(U, S, frame)
            S.cube( [~fbad] )

    return solver.UNDEF

def safety_bmc(N, max, symbols=None, filter=filter_underscore, cex=True, verbose=True, print_cex=print_cex):

    if symbols is None:
        symbols = make_symbols(N)

    def start_frame(U, S, frame):
        if verbose:
            print "frame %5d: "%frame,

    def handle_sat(U, S, frame):
        if verbose:
            print "SAT\n"
        if cex:
            print_cex(U, S, symbols, filter=filter)

    def handle_unsat(U, S, frame):
        if verbose:
            print "UNSAT"

    bad = [ ~po[0]^po.sign() for po in N.get_properties() ]
    constr = [ po[0]^po.sign() for po in N.get_constraints() ]

    return simple_safety_bmc(N, bad, constr, max, start_frame=start_frame, handle_sat=handle_sat, handle_unsat=handle_unsat)
