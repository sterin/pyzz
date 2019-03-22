from pyzz import *


def bmc(N, max):

    # create an unroll object with the Flops initialized in the first frame
    U = unroll(N, init=True)

    # create a solver of the unrolled netlist
    S = solver(U.F)

    prop = conjunction( N, N.get_properties() ) # conjunction of the properties
    constr = N.get_constraints() # constraints

    for i in xrange(max):
        print "Frame:", i

        fprop = U[prop, i] # unroll prop to frame i
        S.cube( U[constr, i] ) # unroll the constraits to frame i

        rc = S.solve( ~fprop ) # run the solver

        if rc == solver.SAT:
            print "SAT"
            return solver.SAT

    print "UNDEF"
    return solver.UNDEF


import click # pip install click


@click.command()
@click.argument("aig", type=click.Path(exists=True, dir_okay=False))
@click.option("--max", type=int, default=25)
def main(aig, max):

    N = pyzz.netlist.read_aiger(aig)
    bmc(N, max)


if __name__ == "__main__":
    main()
