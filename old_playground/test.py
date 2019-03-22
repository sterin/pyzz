def build_miters(N1, N2):

    N, (xlat1, xlat2) = pyzz.combine_cones(N1, N2)

    N1_pos = [ xlat1[po[0]] for po in N1.get_POs() ]
    N2_pos = [ xlat2[po[0]] for po in N2.get_POs() ]

    return N, [ f1^f2 for f1, f2 in zip(N1_pos, N2_pos) ]

def CEC(N1, N2):

    # build miters for the POs
    N, miters = build_miters(N1, N2)

    S = pyzz.solver(N)

    for i, f in enumerate(miters):

        rc = S.solve(f)

        if rc==pyzz.solver.UNSAT:
            print 'Output %d is equivalent'%i
        elif rc==pyzz.solver.SAT:
            print 'Output %d is not equivalent'%i
        else:
            print 'Could not prove output %d'%i