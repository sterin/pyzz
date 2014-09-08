# pyzz

`pyzz` is a python interface for [ABC/ZZ](https://bitbucket.org/niklaseen/abc-zz)

# Usage

First import the pyzz module

    import pyzz

The main classes in the module are:

* Netlist - an AIG.
* Wire - a (possibly) complemented AIG node.
* Solver - a SAT solver and a CNF generator combined in one
* Unroll - unrolls a sequential AIG
* WWMap - maps Wire objects to Wires, handling complemented Wire objects correctly.

## Netlist

The `netlist` class represents an AIG. It can be created from an AIGER file

    N = pyzz.netlist.read_aiger('some_aiger_file.aig')

created from scratch

    N = pyzz.netlist()

or copied from a previously created Netlist

    M, xlat = N.copy()

the mapping between the old and the new netlists is stored in `xlat`

The netlist can be saved back into a file

    N.write_aiger('some_new_aiger_file.aig')    

These methods create new wires

* `N.add_PI()` returns a new PI
* `N.add_PO(fanin=None)` returns a new PO. If `fanin` is specified, it is used as the fanin or the PO
* `N.add_Flop(init=None)` returns a new Flop. If `init` is specified it is used as the initial value of the Flop

The constant true wire is returned by calling

    N.get_True()

These methods return information about the AIG

* `N.n_PIs()`, `N.n_POs()`, `N.n_Flops()`, `N.n_Ands()`: return the number of PIs, POs, Flops, and And gates, respectively
* `N.get_PIs()`, `N.get_POs()`, `N.get_Flops()`, `N.get_Ands()`: return an iterator over the PIs, POs, Flops, and And gates, respectively

Properties are constrains are (possibly complemented) wires.

The properties and constraints can be accessed by

* `N.n_properties()`, `N.get_properties()`: returns an iterator over the properties
* `N.n_constrains()`, `N.get_constraints()`: returns an iterator over the constrains

The properties and constraints can be modified by

* `N.clear_properties()`, `N.clear_constraints()`
* `N.add_property()`, `N.add_property()`

## Wire

A wire represents a, possibly complemented, AIG node.

* `~w`: returns the complement of a wire
* `+w`: returns the non-complemented version of a wire
* `w.sign()`: returns whether the wire is complemented
* `w1 & w2`: returns the AND of two wires
* `w1 | w2`: returns the AND of two wires
* `w1 ^ w2`: returns the AND of two wires
* `w1.implies(w2)`: returns a wire that is true if `w1` implies `w2`
* `w1.equals(w2)`: returns a wire that is true if `w1` equals `w2`
* `w_if.ite(w_then, w_else)`: returns the ITE of the three wires

A wire can be queries to see what type of node it represents

* `w.is_PI()`: returns true if the wire is a PI
* `w.is_PO()`: returns true if the wire is a PO
* `w.is_Flop()`: returns true if the wire is a Flop
* `w.is_And()`: returns true if the wire is an AND gate

If the wire is an AND gate, accessing its fanins is done by

* `w[0]`: returns the left fanin
* `w[1]`: returns the right fanin

I the wire is a PO, its fanin in can be accessed using

* `w[0]`: returns the fanin of the PO
* `w[0]=u`: sets the fanin of the PO

If the wire is a Flop, its next-state function can accessed using

* `w[0]`: returns the next-state function of the Flop
* `w[0]=u`: sets the next-state function of the Flop

## Solver

A Solver object combines a SAT solver and a CNF generator. It can handle wires directly--the cone of the wires is then converted into CNF.

A solver is construct over a netlist:

    S = pyzz.solver(N)

A SAT query is done by using the `solve` method

    res = S.solve()

the `solve` method can be given assumptions

    res = S.solve( wassumption1, wassumption2, ... )

The result of the `solve` method can be one of 

* `pyzz.netlist.UNSAT`: the query was UNSAT
* `pyzz.netlist.SAT`: the query was SAT
* `pyzz.netlist.UNKNOWN`: the query was terminated due to timeout
* `pyzz.netlist.ERROR`: the query was terminated due to error

If the result is SAT, the values of the CEX can be queries by

    v = S[w]

The result of the result can be one of 

* `pyzz.netlist.l_True`: the value of `w` in the satisfying assignment is true
* `pyzz.netlist.l_False`: the value of `w` in the satisfying assignment is false
* `pyzz.netlist.l_Unknown`: the value of `w` in the satisfying assignment is not specified (e.g. the wire was not in the cone of the query)

Additional constraints can be added to the solver

* `S.clause( wires )`: adds the clause `wires` to the solver
* `S.cube( wires )`: adds the cube `wires` to the solver (the same as adding each wire as a unit clause)
* `S.implication( w1, w2 )`: adds `w1` implies `w2` to the solver
* `S.equivalence( w1, w2 )`: adds the`w1` iff `w2` to the solver

## Unroll

The Unroll object unrolls a netlist in time.

The unroll object is created over a netlist

    U = pyzz.unroll(N)

By default, the Flops are initialized in the first frame. It can be modified to implement algorithms that requires that (e.g. inductive step)

    U = pyzz.unroll(N, init=False)

The unroll object has two data members

* `U.N`: the netlist to be unrolled
* `U.F`: the unrolled netlist (F stands for frames)

To unroll a wire use

    fw = U[w,i]

`fw` is a wire in `U.F` that represents `w` at time `i`. If `w` is a sequence of wires, then `fw` is a sequence of wires.

When a wire is unrolled, all of its cone is unrolled as required to represent `w` at time `i`.

To check if a wire is already unrolled to a specific time 

    is_unrolled = (w,i) in U:

The number of unrolled frames (for the wire that is unrolled the most) can be checked using

    len(U)

## Utilities

Boolean operation over a more than two wire:

* `pyzz.conjunction(N, wires)`: returns the conjunction of all the wires in `wires`
* `pyzz.disjunction(N, wires)`: returns the conjunction of all the wires in `wires`
* `pyzz.equals(N, wires1, wires2)`: retuns a wire that is true if the values of the wires in `wires1` equals the respective values in `wires2`

Functions for compositions:

* `pyzz.copy_cone(N_src, N_dst, wires, stop_at={})`: copies the cone of `wires` from the source to the destination netlist. `stop_at` is a mapping from nodes in the source netlist to the destination netlist. If during the traversal of the source netlist a wire in `stop_at` is encountered, it is replaced by the wire it is mapped to.

# Examples

A simple example that writes an AIG with to PIs, and the PO is the AND of these two PIs:

    :::python
    from pyzz import *
    
    N = netlist() # construct a netlist
    
    w1 = N.add_PI() # create a new PI
    w2 = N.add_PI() # create a new PI

    po1 = N.add_PO() # create a new PO
    po1[0] = w1 & w2 # set the fanin of the PO to w1&w2

    po2 = N.add_PO(fanin=w1&w2) # similar to above, but sets the fanin during construction

    N.write_aiger('test1.aig')

Another example, but this time using utility functions:

    :::python
    from pyzz import *
    
    N = netlist() # construct a netlist
    
    wires = [ N.add_PI() for _ in xrange(10) ] # create 10 new PIs

    po2 = N.add_PO(fanin=conjunction(N, wires)) # creates a new PO whose fanin is the conjunction of all the PIs
    
    N.write_aiger('test2.aig')

A simple combinational equivalence checker:

    :::python
    import pyzz
    
    # copy N1 and N2 to a new network, with a sinlge set of PIs for both
    def pre_miter(N1, N2):
    
        # ensure that both netlists have the same number of PIs
        assert N1.n_PIs()==N2.n_PIs()
    
        # ensure that both netlists have the same number of POs
        assert N1.n_POs()==N2.n_POs()
    
        # make sure that both neslists are combinational
        assert N1.n_Flops()==N2.n_Flops()==0
    
        # create the new miter netlist
        N = pyzz.netlist()
    
        # create PIs
        pis = [ N.add_PI() for _ in xrange(N1.n_PIs()) ]
    
        # copy the cone of th POs from N1 to N, using 'pis' to replace the original PIs
        xlat1 = pyzz.copy_cone(N1, N, [po[0] for po in N1.get_POs()], stop_at=dict(zip(N1.get_PIs(), pis)))
    
        # copy the cone of th POs from N2 to N, using 'pis' to replace the original PIs
        xlat2 = pyzz.copy_cone(N2, N, [po[0] for po in N2.get_POs()], stop_at=dict(zip(N2.get_PIs(), pis)))
    
        # return the netlist a list of wire pairs. each pair is the copy of an PO from N1 and N2
        return N, zip( (xlat1[po[0]] for po in N1.get_POs()), (xlat2[po[0]] for po in N2.get_POs()) )
    
    # build miters for each PO pair
    def build_miters(N1, N2):
    
        N, pairs = pre_miter(N1, N2)
        return N, [ f1^f2  for f1, f2 in pairs ]
    
    def CEC(N1, N2):
    
        # build miters for the POs
        N, miters = build_miters(N1, N2)
    
        S = pyzz.solver(N)
    
        for f in miters:
            yield S.solve(f), S, N

A simple BMC

    :::python
    def bmc(N, max):
    
        # create an unroll object with the Flops initialized in the first frame
        U = unroll(N, init=True)
    
        # create a solver of the unrolled netlist
        S = solver(U.F)
    
        prop = conjunction( N, N.get_properties() ) # conjunction of the properties
        constr = conjunction( N, N.get_constraints() ) # conjunction of the constraints
    
        for i in xrange(max):
    
            fprop = U[prop, i] # unroll prop to frame i
            S.cube( U[constr, i] ) # unroll the constraits to frame i
    
            rc = S.solve( ~prop ) # run the solver
    
            if rc == solver.SAT:
                return solver.SAT
    
        return solver.UNDEF