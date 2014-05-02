
#include "pyzzwrapper.h"

#include "ZZ_Bip.hh"

#include <structmember.h>


namespace pyzz
{

void
fill_gates(ZZ::Vec<ZZ::Wire>& vec, ZZ::NetlistRef N, ZZ::GateType type)
{
    For_Gatetype(N, type, w)
    {
        vec.push(w);
    }
}


PyObject* parse_error;
PyObject* aiger_parse_error;
PyObject* zz_error;

using namespace py;

Lit::Lit(ZZ::Lit ll) :
    l(ll)
{
}

Lit::~Lit()
{
}

ref<PyObject>
Lit::tp_repr()
{
    ZZ::String s;
    s += l;

    return String_FromString(s.c_str());
}

void
Lit::initialize(PyObject* module)
{
    _type.tp_repr = wrappers::reprfunc<Lit>;
    _type.tp_compare = wrappers::cmpfunc<Lit,&Lit::tp_compare>;

    _type.tp_flags |= Py_TPFLAGS_CHECKTYPES;

    static PyNumberMethods as_number =
        { 0 };

    as_number.nb_nonzero = wrappers::inquiry<Lit, &Lit::nb_nonzero>;
    as_number.nb_invert = wrappers::unaryfunc<Lit, &Lit::nb_invert>;
    as_number.nb_positive = wrappers::unaryfunc<Lit, &Lit::nb_positive>;

    _type.tp_as_number = &as_number;

    static PyMethodDef methods[] = {

        PYTHONWRAPPER_METH_NOARGS(Lit, id, 0, ""),
        PYTHONWRAPPER_METH_NOARGS(Lit, sign, 0, ""),
        { NULL }  // sentinel
    };

    _type.tp_methods = methods;

    base::initialize("_pyzz.lit");
    add_to_module(module, "lit");
}

int
Lit::tp_compare(PyObject* rhs)
{
    Lit& wrhs = Lit::ensure(rhs);

    if ( l < wrhs.l )
    {
        return -1;
    }

    if ( l > wrhs.l )
    {
        return 1;
    }

    return 0;
}

bool
Lit::nb_nonzero()
{
    return bool(l);
}

ref<PyObject>
Lit::nb_invert()
{
    return build(~l);
}

ref<PyObject>
Lit::nb_positive()
{
    return build(+l);
}

ref<PyObject>
Lit::id()
{
    return Bool_FromLong( l.id );
}

ref<PyObject>
Lit::sign()
{
    return Bool_FromLong( l.sign );
}

ZZ::Lit& Lit::val()
{
    return l;
}

Wire::Wire(ZZ::Wire ww) :
    w(ww)
{
}

Wire::~Wire()
{
}

ref<PyObject>
Wire::tp_repr()
{
    ZZ::String s;
    s += w;

    return String_FromString(s.c_str());
}

void
Wire::initialize(PyObject* module)
{
    _type.tp_repr = wrappers::reprfunc<Wire>;
    _type.tp_compare = wrappers::cmpfunc<Wire,&Wire::tp_compare>;
    _type.tp_hash = wrappers::hashfunc<Wire,&Wire::tp_hash>;

    _type.tp_flags |= Py_TPFLAGS_CHECKTYPES;

    static PyMappingMethods as_mapping = { 0 };

    as_mapping.mp_subscript = wrappers::binaryfunc<Wire, &Wire::mp_subscript>;
    as_mapping.mp_ass_subscript = wrappers::mp_ass_subscript<Wire, &Wire::mp_ass_subscript>;

    _type.tp_as_mapping = &as_mapping;

    static PyNumberMethods as_number =
        { 0 };

    as_number.nb_nonzero = wrappers::inquiry<Wire, &Wire::nb_nonzero>;
    as_number.nb_invert = wrappers::unaryfunc<Wire, &Wire::nb_invert>;
    as_number.nb_positive = wrappers::unaryfunc<Wire, &Wire::nb_positive>;
    as_number.nb_and = wrappers::binaryfunc<Wire, &Wire::nb_and>;
    as_number.nb_or = wrappers::binaryfunc<Wire, &Wire::nb_or>;
    as_number.nb_xor = wrappers::binaryfunc<Wire, &Wire::nb_xor>;

    _type.tp_as_number = &as_number;

    static PyMethodDef methods[] = {

        PYTHONWRAPPER_METH_NOARGS(Wire, id, 0, ""),
        PYTHONWRAPPER_METH_NOARGS(Wire, sign, 0, ""),

        PYTHONWRAPPER_METH_NOARGS(Wire, is_True, 0, ""),
        PYTHONWRAPPER_METH_NOARGS(Wire, is_PI, 0, ""),
        PYTHONWRAPPER_METH_NOARGS(Wire, is_And, 0, ""),
        PYTHONWRAPPER_METH_NOARGS(Wire, is_Flop, 0, ""),
        PYTHONWRAPPER_METH_NOARGS(Wire, is_PO, 0, ""),

        PYTHONWRAPPER_METH_O(Wire, implies, 0, ""),
        PYTHONWRAPPER_METH_VARARGS(Wire, ite, 0, ""),

        { NULL }  // sentinel
    };

    _type.tp_methods = methods;

    base::initialize("_pyzz.wire");
    add_to_module(module, "wire");
}

int
Wire::tp_compare(PyObject* rhs)
{
    Wire& wrhs = Wire::ensure(rhs);

    if( w.nl() != wrhs.w.nl() )
    {
        throw exception::format(PyExc_ValueError, "Wire::tp_compare(): cannot compare Wires of different Netlist objects");
    }

    if ( w < wrhs.w )
    {
        return -1;
    }

    if ( w > wrhs.w )
    {
        return 1;
    }

    return 0;
}

bool
Wire::nb_nonzero()
{
    return bool(w);
}

ref<PyObject>
Wire::nb_invert()
{
    return build(~w);
}

ref<PyObject>
Wire::nb_positive()
{
    return build(+w);
}

ref<PyObject>
Wire::nb_and(PyObject* o)
{
    Wire& rhs = ensure(o);

    return build(s_And(w, rhs.w));
}

ref<PyObject>
Wire::nb_or(PyObject* o)
{
    Wire& rhs = Wire::ensure(o);

    return build(s_Or(w, rhs.w));
}

ref<PyObject>
Wire::nb_xor(PyObject* o)
{
    if ( Wire::check(o) )
    {
        Wire& rhs = Wire::ensure(o);
        return build(s_Xor(w, rhs.w));
    }

    if( !Int_Check(o) )
    {
        throw exception( PyExc_TypeError );
    }

    return build( w ^ bool(Int_AsLong(o)) );
}

ref<PyObject>
Wire::implies(PyObject* o)
{
    Wire& rhs = Wire::ensure(o);
    return build( s_Or( ~w, rhs.w) ) ;
}

ref<PyObject>
Wire::ite(PyObject* args)
{
    borrowed_ref<PyObject> T, E;
    Arg_ParseTuple(args, "OO", &T, &E);

    Wire& wT = Wire::ensure(T);
    Wire& wE = Wire::ensure(E);

    return build( s_Or( s_And(w, wT.w), s_And(~w, wE.w) ) ) ;
}

ref<PyObject>
Wire::is_True()
{
    return Bool_FromLong( w == netlist(w).True() );
}

ref<PyObject>
Wire::is_PI()
{
    return Bool_FromLong( type(w) == ZZ::gate_PI );
}

ref<PyObject>
Wire::is_And()
{
    return Bool_FromLong( type(w) == ZZ::gate_And );
}

ref<PyObject>
Wire::is_Flop()
{
    return Bool_FromLong( type(w) == ZZ::gate_Flop );
}

ref<PyObject>
Wire::is_PO()
{
    return Bool_FromLong( type(w) == ZZ::gate_PO );
}

ref<PyObject>
Wire::id()
{
    return Bool_FromLong( w.id() );
}

ref<PyObject>
Wire::sign()
{
    return Bool_FromLong( w.sign() );
}

ZZ::Wire& Wire::val()
{
    return w;
}

ref<PyObject>
Wire::mp_subscript(PyObject* o)
{
    return build( w[ Int_AsLong(o) ] );
}

void
Wire::mp_ass_subscript(PyObject* key, PyObject* val)
{
    int i = Int_AsLong(key);
    Wire& ww = Wire::ensure(val);

    w.set(i, ww.w);
}

long
Wire::tp_hash()
{
    // TODO: make sure a negative number is not returned.
    return w.hash();
}

void
Netlist::assure_pobs()
{
    ZZ::Assure_Pob0(N, strash);
    ZZ::Assure_Pob0(N, flop_init);
    ZZ::Assure_Pob0(N, properties);
    ZZ::Assure_Pob0(N, constraints);
    ZZ::Assure_Pob0(N, fair_properties);
    ZZ::Assure_Pob0(N, fair_constraints);
}

Netlist::Netlist(bool empty)
{
    if( ! empty )
    {
        assure_pobs();
    }
}

Netlist::~Netlist()
{
}

void
Netlist::initialize(PyObject* module)
{
    static PyMethodDef methods[] = {

        PYTHONWRAPPER_METH_O( Netlist, read_aiger, METH_STATIC, ""),
        PYTHONWRAPPER_METH_O( Netlist, read, METH_STATIC, ""),

        PYTHONWRAPPER_METH_O( Netlist, write_aiger, 0, ""),
        PYTHONWRAPPER_METH_O( Netlist, write, 0, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, get_True, 0, ""),

        PYTHONWRAPPER_METH_VARARGS( Netlist, add_PI, 0, ""),
        PYTHONWRAPPER_METH_VARARGS( Netlist, add_PO, 0, ""),
        PYTHONWRAPPER_METH_VARARGS( Netlist, add_Flop, 0, ""),

        PYTHONWRAPPER_METH_VARARGS( Netlist, add_property, 0, ""),
        PYTHONWRAPPER_METH_VARARGS( Netlist, add_constraint, 0, ""),
        PYTHONWRAPPER_METH_VARARGS( Netlist, add_fair_property, 0, ""),
        PYTHONWRAPPER_METH_VARARGS( Netlist, add_fair_constraint, 0, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, get_PIs, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, get_POs, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, get_Flops, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, get_Ands, 0, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, get_properties, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, get_constraints, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, get_fair_properties, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, get_fair_constraints, 0, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, n_PIs, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, n_POs, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, n_Flops, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, n_Ands, 0, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, n_properties, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, n_constraints, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, n_fair_properties, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, n_fair_constraints, 0, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, copy, 0, ""),

        PYTHONWRAPPER_METH_O( Netlist, uporder, 0, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, remove_unreach, 0, ""),

        { NULL }  // sentinel
    };

    _type.tp_methods = methods;

    static PyGetSetDef getset[] = {
        PYTHONWRAPPER_GETTER(flop_init, Netlist, get_flop_init, ""),
        { NULL } // sentinel
    };

    _type.tp_getset = getset;


    base::initialize("pyzz.netlist");
    add_to_module(module, "netlist");
}

ref<PyObject>
Netlist::read_aiger(PyObject* o)
{
    ref<Netlist> NN = Netlist::build(true);
    ZZ::readAigerFile( String_AsString(o), NN->N);
    NN->assure_pobs();
    return NN;
}

ref<PyObject>
Netlist::read(PyObject* o)
{
    ref<Netlist> NN = Netlist::build(true);
    NN->N.read( String_AsString(o) );
    NN->assure_pobs();
    return NN;
}

void
Netlist::write_aiger(PyObject* o)
{
    writeAigerFile( String_AsString(o) , N);
}

void
Netlist::write(PyObject* o)
{
    N.write( String_AsString(o) );
}

ref<PyObject>
Netlist::get_True()
{
    return Wire::build(N.True());
}

ref<PyObject>
Netlist::add_PI(PyObject* args)
{
    int id = N.typeCount(ZZ::gate_PI);

    Arg_ParseTuple(args, "|i", &id);

    return Wire::build(N.add(ZZ::PI_(id)));
}

ref<PyObject>
Netlist::add_PO(PyObject* args)
{
    int id = N.typeCount(ZZ::gate_PO);

    Arg_ParseTuple(args, "|i", &id);

    return Wire::build(N.add(ZZ::PO_(id)));
}

ref<PyObject>
Netlist::add_Flop(PyObject* args)
{
    int id = N.typeCount(ZZ::gate_Flop);

    Arg_ParseTuple(args, "|i", &id);

    ZZ::Wire ff = N.add(ZZ::Flop_(id));

    ZZ::Get_Pob(N, flop_init);
    flop_init(ff) = ZZ::l_False;

    return Wire::build(ff);
}

ref<PyObject>
Netlist::add_property(PyObject* o)
{
    Wire& w = Wire::ensure(o);

    ZZ::Get_Pob(N, properties);
    properties.push(w.w);
}

ref<PyObject>
Netlist::add_constraint(PyObject* o)
{
    Wire& w = Wire::ensure(o);

    ZZ::Get_Pob(N, constraints);
    constraints.push(w.w);
}


ref<PyObject>
Netlist::add_fair_property(PyObject* o)
{
    ZZ::Vec<ZZ::Wire> fcs;
    zzvec_from_iter<Wire>(fcs, o);

    ZZ::Get_Pob(N, fair_properties);
    fair_properties.push();
    fcs.copyTo(fair_properties.last());
}

ref<PyObject>
Netlist::add_fair_constraint(PyObject* o)
{
    Wire& w = Wire::ensure(o);

    ZZ::Get_Pob(N, fair_constraints);
    fair_constraints.push(w.w);
}
void
Netlist::remove_unreach()
{
    removeUnreach(N);
}

ref<PyObject>
Netlist::n_PIs()
{
    return Int_FromSize_t(N.typeCount(ZZ::gate_PI));
}

ref<PyObject>
Netlist::n_POs()
{
    return Int_FromSize_t(N.typeCount(ZZ::gate_PO));
}

ref<PyObject>
Netlist::n_Flops()
{
    return Int_FromSize_t(N.typeCount(ZZ::gate_Flop));
}

ref<PyObject>
Netlist::n_Ands()
{
    return Int_FromSize_t(N.typeCount(ZZ::gate_And));
}


ref<PyObject>
Netlist::n_properties()
{
    ZZ::Get_Pob(N, properties);
    return Int_FromLong( properties.size() );
}

ref<PyObject>
Netlist::n_constraints()
{
    ZZ::Get_Pob(N, constraints);
    return Int_FromLong( constraints.size() );
}

ref<PyObject>
Netlist::n_fair_properties()
{
    ZZ::Get_Pob(N, fair_properties);
    return Int_FromLong( fair_properties.size() );
}

ref<PyObject>
Netlist::n_fair_constraints()
{
    ZZ::Get_Pob(N, fair_constraints);
    return Int_FromLong( fair_constraints.size() );
}

ref<PyObject>
Netlist::get_PIs()
{
    ZZ::Vec<ZZ::Wire> vec;

    fill_gates(vec, N, ZZ::gate_PI);

    return VecIterator<Wire>::build(vec);
}

ref<PyObject>
Netlist::get_POs()
{
    ZZ::Vec<ZZ::Wire> vec;

    fill_gates(vec, N, ZZ::gate_PO);

    return VecIterator<Wire>::build(vec);
}

ref<PyObject>
Netlist::get_Flops()
{
    ZZ::Vec<ZZ::Wire> vec;

    fill_gates(vec, N, ZZ::gate_Flop);

    return VecIterator<Wire>::build(vec);
}

ref<PyObject>
Netlist::get_Ands()
{
    ZZ::Vec<ZZ::Wire> vec;

    fill_gates(vec, N, ZZ::gate_And);

    return VecIterator<Wire>::build(vec);
}

ref<PyObject>
Netlist::get_properties()
{
    ZZ::Get_Pob(N, properties);

    ZZ::Vec<ZZ::Wire> props;
    properties.copyTo(props);

    return VecIterator<Wire>::build(props);
}


ref<PyObject>
Netlist::get_constraints()
{
    ZZ::Get_Pob(N, constraints);

    ZZ::Vec<ZZ::Wire> props;
    constraints.copyTo(props);

    return VecIterator<Wire>::build(props);
}

ref<PyObject>
Netlist::get_fair_properties()
{
    ZZ::Get_Pob(N, fair_properties);

    ref<PyObject> list=List_New(fair_properties.size());

    ZZ::Vec<ZZ::Wire> props;

    for( uind i=0; i<fair_properties.size() ; i++)
    {
        fair_properties[i].copyTo(props);
        List_SetItem(list, i, VecIterator<Wire>::build(props));
    }

    return list;
}

ref<PyObject>
Netlist::get_fair_constraints()
{
    ZZ::Get_Pob(N, fair_constraints);

    ZZ::Vec<ZZ::Wire> props;
    fair_constraints.copyTo(props);

    return VecIterator<Wire>::build(props);
}

ref<PyObject>
Netlist::get_flop_init()
{
    ZZ::Get_Pob(N, flop_init);
    borrowed_ref<Netlist> pp(this);
    return FlopInitMap::build(flop_init, pp);
}

ref<PyObject>
Netlist::copy()
{
    ref<Netlist> CN = build();

    ZZ::Netlist& M = CN->N;

    ZZ::WWMap xlat;
    xlat( N.True() ) = M.True();

    For_Gatetype(N, ZZ::gate_PI, pi)
    {
        int id = M.typeCount(ZZ::gate_PI);
        xlat(pi) = M.add(ZZ::PI_(id));
    }

    ZZ::Add_Pob(M, flop_init);

    For_Gatetype(N, ZZ::gate_Flop, ff)
    {
        int id = M.typeCount(ZZ::gate_Flop);
        ZZ::Wire mff = M.add(ZZ::Flop_(id));
        flop_init(mff) = ZZ::l_False;
        xlat(ff) = mff;
    }

    For_Gatetype(N, ZZ::gate_And, g)
    {
        ZZ::Wire g0 = M[xlat[g[0]]];
        ZZ::Wire g1 = M[xlat[g[1]]];
        xlat(g) = ZZ::s_And(g0, g1);
    }

    For_Gatetype(N, ZZ::gate_Flop, ff)
    {
        ZZ::Wire w = M[xlat[ff]];
        w.set(0, xlat[ff[0]]);
    }

    For_Gatetype(N, ZZ::gate_PO, po)
    {
        int id = M.typeCount(ZZ::gate_PO);

        ZZ::Wire mpo = M.add(ZZ::PO_(id));
        mpo.set(0, xlat[po[0]]);
    }

    return CN;
}

ref<PyObject>
Netlist::uporder(PyObject* seq)
{
    ZZ::Vec<ZZ::Wire> sinks;
    zzvec_from_iter<Wire>(sinks, seq);

    assert( sinks.size() > 0 );

    ZZ::Vec<ZZ::gate_id> order;
    ZZ::upOrder(sinks, order, false);

    ZZ::Vec<ZZ::Wire> worder;

    for(uind i=0 ; i<order.size() ; i++)
    {
        worder.push( N[order[i]] );
    }

    return VecIterator<Wire>::build(worder);
}


Solver::Solver(ZZ::NetlistRef N) :
    _callback(None),
    _fail(false),
    _conflict(None),
    _N(N),
    _C(_S, _N, _wtos, _keep)
{
    _C.initKeep();
    _S.timeout_cb = sat_callback;
    _S.timeout_cb_data = this;
}

Solver::~Solver()
{
}

PyObject*
Solver::base_construct(PyTypeObject *subtype, PyObject *args, PyObject *kwds)
{
    return _type.tp_alloc(subtype, 0);
}

void
Solver::construct(Solver* p, PyObject* args, PyObject*)
{
    borrowed_ref<PyObject> pN;
    Arg_ParseTuple(args, "O", &pN);

    Netlist& N = Netlist::ensure(pN);

    new (p) Solver(N.N);
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
        PYTHONWRAPPER_GETSET(callback, Solver, _callback, _callback, "SAT callback"),
        PYTHONWRAPPER_GETTER(conflict, Solver, _conflict, "conflict clause (in terms of assumptions)"),
        PYTHONWRAPPER_GETSET(timeout, Solver, get_timeout, set_timeout, "SAT timeout"),
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
        String_FromString("l_Error"),
        readonly_static_descriptor::build( Int_FromLong( ZZ::l_Error.value )).get_cast<PyObject>()
    );

    Dict_SetItem(
        dict,
        String_FromString("l_False"),
        readonly_static_descriptor::build( Int_FromLong( ZZ::l_False.value )).get_cast<PyObject>()
    );

    Dict_SetItem(
        dict,
        String_FromString("l_True"),
        readonly_static_descriptor::build( Int_FromLong( ZZ::l_True.value )).get_cast<PyObject>()
    );

    _type.tp_dict = dict.release();

    base::initialize("_pyzz.solver");
    add_to_module(module, "solver");
}

struct lbool_proxy
{
    typedef ZZ::lbool zztype;

    static ref<PyObject> build(const ZZ::lbool& v)
    {
        return Int_FromLong(v.value);
    }
};

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

    return VecIterator<lbool_proxy>::build(vec);
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
    static char *kwlist[] = { "control", NULL };

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
    static char *kwlist[] = { "control", NULL };

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

    _fail = false;
    _conflict = None;

    ZZ::lbool res;

    {
        enable_threads threads;
        res = _S.solve(assumptions);
    }

    if (_fail)
    {
        exception::check();
    }

    if ( res == ZZ::l_False )
    {
        _conflict = List_New(_S.conflict.size());

        for(uind i=0; i<_S.conflict.size() ; i++)
        {
            ZZ::Lit l = _S.conflict[i];

            uind* idx;
            bool found  = lit_to_ass.get(l, idx);
            assert( found );

            borrowed_ref<PyObject> a = List_GetItem(py_assumptions, *idx);
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

static int py_to_lbool(borrowed_ref<PyObject> o)
{
    if( Int_Check(o) )
    {
        long l = Int_AsLong(o);

        if( l!=0 )
        {
            return 1;
        }

        return 0;
    }

    return -1;
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
Solver::set_timeout(borrowed_ref<PyObject> timeout)
{
    _S.timeout = Int_AsUnsignedLongLongMask(timeout);
}

ref<PyObject>
Solver::get_timeout()
{
    return Long_FromUnsignedLongLong(static_cast<unsigned PY_LONG_LONG>(_S.timeout));
}

void
Solver::set_callback(borrowed_ref<PyObject> timeout)
{
    _callback = timeout;
}

ref<PyObject>
Solver::get_callback()
{
    return _callback;
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


bool
Solver::sat_callback(uint64 work)
{
    if (_callback && _callback != None)
    {
        return Object_IsTrue( Object_CallFunction(_callback, "K", work) );
    }

    return true;
}

bool
Solver::sat_callback(uint64 work, void* data)
{
    Solver* s = reinterpret_cast<Solver*>(data);

    {
        gil_state_ensure gil;

        try
        {
            return s->sat_callback(work);
        }
        PYTHONWRAPPER_CATCH
    }

    s->_fail = true;
    return false;
}


Unroll::Unroll() :
    _init(true),
    _next(0)
{
}

Unroll::~Unroll()
{
}

void
Unroll::initialize(PyObject* module)
{
    _type.tp_init = wrappers::initproc<Unroll>;

    static PyMappingMethods as_mapping = { 0 };

    as_mapping.mp_subscript = wrappers::binaryfunc<Unroll, &Unroll::mp_subscript>;

    _type.tp_as_mapping = &as_mapping;

    static PyMethodDef methods[] = {

        PYTHONWRAPPER_METH_O(Unroll, unroll, 0, "Unroll frames"),
        { NULL }  // sentinel
    };

    _type.tp_methods = methods;

    static PyGetSetDef getset[] = {
        PYTHONWRAPPER_GETTER(N, Unroll, _N, "Netlist"),
        PYTHONWRAPPER_GETTER(F, Unroll, _F, "Unrolled netlist"),
        { NULL } // sentinel
    };

    _type.tp_getset = getset;

    static PyMemberDef members[] = {
        { "frames", T_INT, offsetof(Unroll, _next), READONLY, "number of unrolled frames"},
        {NULL}  // sentinel
    };

    _type.tp_members = members;

    base::initialize("_pyzz.unroll");
    add_to_module(module, "unroll");
}

void
Unroll::tp_init(PyObject* args, PyObject* kwds)
{
    static char *kwlist[] = { "N", "unroll", "init", NULL };

    borrowed_ref<PyObject> pN;
    borrowed_ref<PyObject> pyunroll;
    borrowed_ref<PyObject> init = True;

    Arg_ParseTupleAndKeywords(args, kwds, "O|OO", kwlist, &pN, &pyunroll, &init);

    N = Netlist::ensure(pN).N;
    _N = pN;

    ref<Netlist> pF = Netlist::build();
    F = pF->N;
    _F = pF;

    _init = Object_IsTrue(init);

    if( pyunroll )
    {
        unroll(pyunroll);
    }
}

ZZ::Wire
Unroll::unroll_wire(ZZ::Wire w, int k)
{
    ZZ::WWMap& M = _maps[k];

    if( ZZ::type(w) == ZZ::gate_And )
    {
        return ZZ::s_And( F[M[w[0]]], F[M[w[1]]] );
    }
    else if( ZZ::type(w) == ZZ::gate_PI )
    {
        return F.add( ZZ::PI_( F.typeCount(ZZ::gate_PI) ) );
    }
    else if( ZZ::type(w) == ZZ::gate_Flop )
    {
        if( k==0 )
        {
            if( _init )
            {
                ZZ::Get_Pob(N, flop_init);
                ZZ::lbool init_val = flop_init[w];

                if( init_val == ZZ::l_True)
                {
                    return F.True();
                }
                else if ( init_val == ZZ::l_False)
                {
                    return ~F.True();
                }
            }

            return F.add( ZZ::PI_( F.typeCount(ZZ::gate_PI) ) );
        }

        return F[_maps[k-1][w[0]]];
    }
    else if( ZZ::type(w) == ZZ::gate_PO )
    {
        ZZ::Wire f = F.add( ZZ::PO_( F.typeCount(ZZ::gate_PO) ) );
        f.set(0, F[M[w[0]]] );

        return f;
    }

    assert( false );
}

void Unroll::unroll_frame()
{
    _maps.push();
    assert( _maps.size() == (_next+1) );

    ZZ::WWMap& M = _maps[_next];
    M(N.True()) = F.True();

    ZZ::Vec<ZZ::Wire> sinks;

    fill_gates(sinks, N, ZZ::gate_PO);
    fill_gates(sinks, N, ZZ::gate_Flop);
    fill_gates(sinks, N, ZZ::gate_PI);
    fill_gates(sinks, N, ZZ::gate_And);

    ZZ::Vec<ZZ::gate_id> order;
    ZZ::upOrder(sinks, order, false);

    for(uind i=0 ; i<order.size() ; i++)
    {
        ZZ::Wire w = N[order[i]];
        M(w) = unroll_wire(w, _next);
    }

    _next++;
}

ref<PyObject>
Unroll::unroll(PyObject* o)
{
    const int k = Int_AsLong(o);

    for( int i=0 ; i<k ; i++)
    {
        unroll_frame();
    }

    return borrow(this);
}

ref<PyObject>
Unroll::mp_subscript(PyObject* args)
{
    int k;
    borrowed_ref<PyObject> o;

    Arg_ParseTuple(args, "Oi", &o, &k);

    ZZ::WWMap& M = _maps[k];

    try
    {
        ZZ::Vec<ZZ::Wire> vec;

        pywrapper_for_iterator(o, pw)
        {
            Wire& w = Wire::ensure(pw);
            vec.push( F[M[w.w]] );
        }

        return VecIterator<Wire>::build(vec);
    }
    catch( exception& e )
    {
        exception::handle(PyExc_TypeError);
    }

    Wire& w = Wire::ensure(o);
    return Wire::build( F[M[w.w]] );
}

ref<PyObject>
imc(PyObject* args, PyObject* kwds)
{
    static char *kwlist[] = { "N", "props", "first_k", "quiet", NULL };

    borrowed_ref<PyObject> pN;
    borrowed_ref<PyObject> pyprops;
    int first_k = 0;
    int quiet = 1;
    Arg_ParseTupleAndKeywords(args, kwds, "O|Oii:imc", kwlist, &pN, &pyprops, &first_k, &quiet);

    if( first_k < 0)
    {
        first_k = 0;
    }

    Netlist& N = Netlist::ensure(pN);

    ZZ::Vec<ZZ::Wire> props;

    if( pyprops )
    {
        pywrapper_for_iterator(pyprops, pyitem)
        {
            Wire& ww = Wire::ensure(pyitem);
            props.push(ww.w);
        }
    }
    else
    {
        ZZ::Vec<ZZ::Wire> pos;
        fill_gates(pos, N.N, ZZ::gate_PO);

        for(uind i=0; i<pos.size() ; i++)
        {
            props.push( ~pos[i] );
        }
    }

    int buf_free_depth = -1;

    ZZ::Params_ImcStd params;

    params.first_k = first_k;
    params.quiet = quiet;

    ZZ::lbool res = ZZ::imcStd(
        N.N,
        props,
        params,
        0,
        ZZ::NetlistRef(),
        &buf_free_depth,
        0
        );

    return BuildValue("ii", res.value, buf_free_depth);
}

void
init()
{
    using namespace py;
    using namespace pyzz;

    ref<PyObject> zz_error = Err_NewException("_pyzz.zz_error", PyExc_Exception, 0);
    ref<PyObject> aiger_parse_error = Err_NewException("_pyzz.aiger_parse_error", zz_error, 0);
    ref<PyObject> parse_error = Err_NewException("_pyzz.parse_error", zz_error, 0);

    pyzz::zz_error = zz_error;
    pyzz::aiger_parse_error = aiger_parse_error;
    pyzz::parse_error = parse_error;

    static PyMethodDef pyzz_methods[] = {
        PYTHONWRAPPER_FUNC_KEYWORDS(imc, 0, "interpolation-based model-checking"),
        { 0 }
    };

    borrowed_ref<PyObject> mod = InitModule4(
        "_pyzz",
        pyzz_methods,
        "Python interface to ZZ and Bip",
        NULL,
        PYTHON_API_VERSION
        );

    Module_AddObject( mod, "zz_error", zz_error );
    Module_AddObject( mod, "aiger_parse_error", aiger_parse_error );
    Module_AddObject( mod, "parse_error", parse_error );

    readonly_static_descriptor::initialize();

    Lit::initialize(mod);
    Wire::initialize(mod);
    WMap<Wire>::initialize(mod,"_pyzz.wwmap", "wwmap");
    WMap<Lit>::initialize(mod,"_pyzz.wlmap", "wlmap");
    FlopInitMap::initialize(mod, "_pyzz.flop_init_map", "flop_init_map");
    VecIterator<Wire>::initialize(mod, "_pyzz.witerator", "witerator");
    VecIterator<lbool_proxy>::initialize(mod, "_pyzz.lbooliterator", "lbooliterator");
    Netlist::initialize(mod);
    Solver::initialize(mod);
    Unroll::initialize(mod);
}

} // namespace pyzz
