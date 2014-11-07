#include "solver.h"

#include "wire.h"
#include "lit.h"
#include "veciterator.h"
#include "netlist.h"

#include "readonly_static_descriptor.h"

#include <limits>

#include <structmember.h>

namespace pyzz
{

using namespace py;

Solver::Solver(ZZ::NetlistRef N) :
    _conflict_limit(static_cast<uint64>(UINT64_MAX)),
    _conflict(None),
    _N(N),
    _C(_S, _N, _wtos, _keep)
{
    _C.quant_claus = true;
}

Solver::~Solver()
{
}

void
Solver::construct(Solver* p, PyObject* args, PyObject* kwds)
{
    static char *kwlist[] = { "N", "conflict_limit", NULL };

    borrowed_ref<PyObject> pN;
    borrowed_ref<PyObject> conflict_limit;

    Arg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &pN, &conflict_limit);

    Netlist& N = Netlist::ensure(pN);

    new (p) Solver(N.N);

    if( conflict_limit )
    {
        p->set_conflict_limit(conflict_limit);
    }
}

void
Solver::initialize(PyObject* module)
{
    static PyMappingMethods as_mapping = { 0 };
    as_mapping.mp_subscript = wrappers::binaryfunc<Solver, &Solver::mp_subscript>;
    _type.tp_as_mapping = &as_mapping;

    static PySequenceMethods as_sequence = { 0 };
    as_sequence.sq_contains = wrappers::objobjproc<Solver, &Solver::sq_contains>;
    _type.tp_as_sequence = &as_sequence;

    static PyMethodDef methods[] = {

        PYTHONWRAPPER_METH_O(Solver, keep, 0, ""),
        PYTHONWRAPPER_METH_O(Solver, clausify, 0, ""),
        PYTHONWRAPPER_METH_KEYWORDS(Solver, clause, 0, "add a clause to the solver"),
        PYTHONWRAPPER_METH_KEYWORDS(Solver, cube, 0, "add a cube to the solver"),
        PYTHONWRAPPER_METH_KEYWORDS(Solver, implication, 0, "add an implication to the solver"),
        PYTHONWRAPPER_METH_KEYWORDS(Solver, equivalence, 0, "add an equivalence to the solver"),
        PYTHONWRAPPER_METH_KEYWORDS(Solver, apply_cex, 0, "apply the cex to wires/literals"),
        PYTHONWRAPPER_METH_NOARGS(Solver, new_var, 0,    "get a new variable"),
        PYTHONWRAPPER_METH_VARARGS(Solver, solve, 0, "solve the SAT instance"),
        PYTHONWRAPPER_METH_O(Solver, value, 0, "get the assignment value for a wire"),
        PYTHONWRAPPER_METH_O(Solver, has_wire, 0, "is the wire inside the solver"),

        { NULL }  // sentinel
    };

    _type.tp_methods = methods;

    static PyGetSetDef getset[] = {
        PYTHONWRAPPER_GETTER(conflict, Solver, _conflict, "conflict clause (in terms of assumptions)"),
        PYTHONWRAPPER_GETSET(conflict_limit, Solver, get_conflict_limit, set_conflict_limit, "SAT timeout"),
        { NULL } // sentinel
    };

    _type.tp_getset = getset;

    static PyMemberDef members[] = {
        {NULL}  // sentinel
    };

    _type.tp_members = members;

    ref<PyObject> dict = Dict_New();

    Dict_SetItem(
        dict,
        String_FromString("l_Undef"),
        readonly_static_descriptor::build( Int_FromLong( ZZ::l_Undef.value )).get_cast<PyObject>()
    );

    Dict_SetItem(
        dict,
        String_FromString("UNDEF"),
        readonly_static_descriptor::build( Int_FromLong( ZZ::l_Undef.value )).get_cast<PyObject>()
    );

    Dict_SetItem(
        dict,
        String_FromString("l_Error"),
        readonly_static_descriptor::build( Int_FromLong( ZZ::l_Error.value )).get_cast<PyObject>()
    );

    Dict_SetItem(
        dict,
        String_FromString("ERROR"),
        readonly_static_descriptor::build( Int_FromLong( ZZ::l_Error.value )).get_cast<PyObject>()
    );

    Dict_SetItem(
        dict,
        String_FromString("l_False"),
        readonly_static_descriptor::build( Int_FromLong( ZZ::l_False.value )).get_cast<PyObject>()
    );

    Dict_SetItem(
        dict,
        String_FromString("UNSAT"),
        readonly_static_descriptor::build( Int_FromLong( ZZ::l_False.value )).get_cast<PyObject>()
    );

    Dict_SetItem(
        dict,
        String_FromString("l_True"),
        readonly_static_descriptor::build( Int_FromLong( ZZ::l_True.value )).get_cast<PyObject>()
    );

    Dict_SetItem(
        dict,
        String_FromString("SAT"),
        readonly_static_descriptor::build( Int_FromLong( ZZ::l_True.value )).get_cast<PyObject>()
    );

    _type.tp_dict = dict.release();

    base::initialize("_pyzz.solver");
    add_to_module(module, "solver");
}

ref<PyObject>
Solver::mp_subscript(PyObject* o)
{
    if( Wire::check(o) || Lit::check(o) )
    {
        return value(o);
    }

    ZZ::Vec<ZZ::lbool> vec;

    pywrapper_for_iterator(o, pw)
    {
        ZZ::lbool vw = _S.value( get_Lit(pw) );
        vec.push( vw );
    }

    return Vec<lbool_proxy>::build(vec);
}

int
Solver::sq_contains(PyObject* pkey)
{
    Wire& ww = Wire::ensure(pkey);
    bool contains = _C.get(ww.w)!=ZZ::Lit_NULL;
    return contains ? 1 : 0;
}

void
Solver::keep(PyObject* seq)
{
    pywrapper_for_iterator(seq, pyitem)
    {
        Wire& ww = Wire::ensure(pyitem);
        _C.keep.add(ww.w);
    }
}

void
Solver::clausify(PyObject* seq)
{
    pywrapper_for_iterator(seq, pyitem)
    {
        Wire& ww = Wire::ensure(pyitem);
        _C.clausify(ww.w);
    }
}

void
Solver::clause(PyObject* args, PyObject* kwds)
{
    static char *kwlist[] = { "clause", "control", NULL };

    borrowed_ref<PyObject> control;
    borrowed_ref<PyObject> iter;

    Arg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &iter, &control);

    ZZ::Vec<ZZ::Lit> clause;

    if (control)
    {
        clause.push(~get_Lit(control));
    }

    pywrapper_for_iterator(iter, pyitem)
    {
        clause.push( get_Lit(pyitem));
    }

    _S.addClause(clause);
}

void
Solver::cube(PyObject* args, PyObject* kwds)
{
    static char *kwlist[] = { "cube", "control", NULL };

    borrowed_ref<PyObject> control;
    borrowed_ref<PyObject> iter;

    Arg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &iter, &control);

    pywrapper_for_iterator(iter, pyitem)
    {
        if (control)
        {
            _S.addClause( ~get_Lit(control), get_Lit(pyitem) );
        }
        else
        {
            _S.addClause( get_Lit(pyitem) );
        }
    }
}

void
Solver::implication(PyObject* args, PyObject* kwds)
{
    static char *kwlist[] = { "a", "b", "control", NULL };

    borrowed_ref<PyObject> control;
    borrowed_ref<PyObject> w1, w2;

    Arg_ParseTupleAndKeywords(args, kwds, "OO|O", kwlist, &w1, &w2, &control);

    ZZ::Lit l1 = get_Lit(w1);
    ZZ::Lit l2 = get_Lit(w2);

    if (control)
    {
        _S.addClause(~get_Lit(control), ~l1, l2);
    }
    else
    {
        _S.addClause(~l1, l2);
    }
}

void
Solver::equivalence(PyObject* args, PyObject* kwds)
{
    static char *kwlist[] = { "a", "b", "control", NULL };

    borrowed_ref<PyObject> control;
    borrowed_ref<PyObject> w1, w2;

    Arg_ParseTupleAndKeywords(args, kwds, "OO|O", kwlist, &w1, &w2, &control);

    ZZ::Lit l1 = get_Lit(w1);
    ZZ::Lit l2 = get_Lit(w2);

    if (control)
    {
        ZZ::Lit lcontrol = get_Lit(control);

        _S.addClause(~lcontrol, ~l1, l2);
        _S.addClause(~lcontrol, ~l2, l1);
    }
    else
    {
        _S.addClause(~l1, l2);
        _S.addClause(~l2, l1);
    }
}

ref<PyObject>
Solver::solve(PyObject* seq)
{
    ref<PyObject> py_assumptions = List_New(0);

    ZZ::Vec<ZZ::Lit> assumptions;
    ZZ::Map<ZZ::Lit,uint> lit_to_ass;

    int i=0;
    pywrapper_for_iterator(seq, pyitem)
    {
        List_Append(py_assumptions, pyitem);
        ZZ::Lit l = get_Lit(pyitem);
        assumptions.push(l);
        lit_to_ass.set(l, i++);
    }

    _conflict = None;

    ZZ::lbool res;

    _S.setConflictLim(_conflict_limit);

    {
        enable_threads threads;
        res = _S.solve(assumptions);
    }

    if ( res == ZZ::l_False )
    {
        ZZ::Vec<ZZ::Lit> conflict;
        _S.getConflict(conflict);

        _conflict = List_New(conflict.size());

        for(uind i=0; i< conflict.size() ; i++)
        {
            ZZ::Lit l = conflict[i];

            uind idx;
            bool found  = lit_to_ass.peek(l, idx);
            assert( found );

            borrowed_ref<PyObject> a = List_GetItem(py_assumptions, idx);
            List_SetItem(_conflict, i, a);
        }
    }

    return Int_FromLong(res.value);
}

ref<PyObject>
Solver::new_var()
{
    return Lit::build( _S.addLit() );
}

ref<PyObject>
Solver::value(PyObject* pw)
{
    ZZ::lbool vw = _S.value( get_Lit(pw) );
    return Int_FromLong(vw.value);
}

ref<PyObject>
Solver::apply_cex(PyObject* args, PyObject* kwds)
{
    static char *kwlist[] = { "seq", "value_map", NULL };

    borrowed_ref<PyObject> seq;
    borrowed_ref<PyObject> value_map;

    Arg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &seq, &value_map);

    int V[4] = { -1, -1, 1, 0 };

    if(value_map)
    {
        ref<PyObject> o = Sequence_Fast(value_map, "");

        if( Sequence_Fast_GET_SIZE(o) != 4)
        {
            throw exception(PyExc_TypeError);
        }

        for( uind i=0 ; i<4; i++)
        {
            V[i] = py_to_lbool(Sequence_Fast_GET_ITEM(o, i));
        }
    }

    ref<PyObject> res = List_New(0);

    pywrapper_for_iterator(seq, pyitem)
    {
        if( Wire::check(pyitem) )
        {
            Wire& ww = Wire::ensure(pyitem);
            ZZ::Lit l = _C.clausify(ww.w);
            ZZ::lbool v = _S.value(l);

            if( v != ZZ::l_Undef )
            {
                ref<PyObject> w = Wire::build( ww.w ^ V[v.value] );
                List_Append(res, w);
            }
        }
        else if( Lit::check(pyitem) )
        {
            Lit& ll = Lit::ensure(pyitem);
            ZZ::lbool v = _S.value(ll.l);

            if( v != ZZ::l_Undef )
            {
                ref<PyObject> l = Lit::build( ll.l ^ V[v.value] );
                List_Append(res, l);
            }
        }
    }

    return res;
}

ref<PyObject>
Solver::has_wire(PyObject* o)
{
    Wire& ww = Wire::ensure(o);
    return Bool_FromLong( _C.get(ww.w) != ZZ::Lit_NULL);
}

void
Solver::set_conflict_limit(borrowed_ref<PyObject> o)
{
    if( o == None )
    {
        _conflict_limit = std::numeric_limits<uint64>::max();
    }
    else if ( Int_Check(o) )
    {
        _conflict_limit = Int_AsLong(o);
    }
    else if ( Long_Check(o) )
    {
        _conflict_limit = Long_AsUnsignedLongLong(o);
    }
    else
    {
        throw exception(PyExc_TypeError);
    }
}

ref<PyObject>
Solver::get_conflict_limit()
{
    if( _conflict_limit == std::numeric_limits<uint64>::max() )
    {
        return None;
    }

    if( _conflict_limit <= std::numeric_limits<long>::max() )
    {
        return Int_FromLong(_conflict_limit);
    }

    return Long_FromUnsignedLongLong(_conflict_limit);
}

ZZ::Lit
Solver::get_Lit(PyObject* o)
{
    if( Wire::check(o) )
    {
        Wire& ww = Wire::ensure(o);
        return _C.clausify(ww.w);
    }

    else if( Lit::check(o) )
    {
        Lit& ll = Lit::ensure(o);
        return ll.l;
    }

    throw exception( PyExc_TypeError );
}

} // namespace pyzz
