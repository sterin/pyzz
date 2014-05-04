#include "netlist.h"
#include "wmap.h"
#include "veciterator.h"

namespace pyzz
{

using namespace py;

class FlopInitMap :
    public type_base<FlopInitMap>
{
public:

    typedef FlopInitMap C;
    typedef ZZ::Pec_LProp<ZZ::lbool,ZZ::gate_Flop> ZC;
    typedef type_base<C> base;

    FlopInitMap(ZC& m, borrowed_ref<PyObject> p) :
        wmap(m),
        _ref(p)
    {
    }

    ~FlopInitMap()
    {
    }

    static void initialize(PyObject* module, const char* mname, const char* name)
    {
        static PyMappingMethods as_mapping = { 0 };

        as_mapping.mp_subscript = wrappers::binaryfunc<C, &C::mp_subscript>;
        as_mapping.mp_ass_subscript = wrappers::mp_ass_subscript<C, &C::mp_ass_subscript>;

        base::_type.tp_as_mapping = &as_mapping;

        base::initialize(mname);
        base::add_to_module(module, name);
    }

    static ZZ::Wire ensure_flop(PyObject* o)
    {
        Wire& key = Wire::ensure(o);

        if( type(key.w) != ZZ::gate_Flop || key.w.sign() )
        {
            throw py::exception(PyExc_KeyError);
        }

        return key.w;
    }

    ref<PyObject> mp_subscript(PyObject* pkey)
    {
        ZZ::Wire key = ensure_flop(pkey);
        ZZ::lbool val = wmap[key];
        return Int_FromLong( val.value );
    }

    void mp_ass_subscript(PyObject* pkey, PyObject* pval)
    {
        ZZ::Wire key = ensure_flop(pkey);
        long val = Int_AsLong(pval);

        if( val <0 || val>3 )
        {
            throw py::exception(PyExc_ValueError);
        }

        wmap(key) = ZZ::lbool_new(val);
    }

public:

    ZC& wmap;
    ref<PyObject> _ref;
};

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
Netlist::assure_pobs()
{
    ZZ::Assure_Pob0(N, strash);
    ZZ::Assure_Pob0(N, flop_init);
    ZZ::Assure_Pob0(N, properties);
    ZZ::Assure_Pob0(N, constraints);
    ZZ::Assure_Pob0(N, fair_properties);
    ZZ::Assure_Pob0(N, fair_constraints);
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
        PYTHONWRAPPER_METH_KEYWORDS( Netlist, add_PO, 0, ""),
        PYTHONWRAPPER_METH_KEYWORDS( Netlist, add_Flop, 0, ""),

        PYTHONWRAPPER_METH_O( Netlist, add_property, 0, ""),
        PYTHONWRAPPER_METH_O( Netlist, add_constraint, 0, ""),
        PYTHONWRAPPER_METH_O( Netlist, add_fair_property, 0, ""),
        PYTHONWRAPPER_METH_O( Netlist, add_fair_constraint, 0, ""),

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

    FlopInitMap::initialize(module, "_pyzz.flop_init_map", "flop_init_map");
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
Netlist::add_PO(PyObject* args, PyObject* kwds)
{
    int id = N.typeCount(ZZ::gate_PO);

    static char *kwlist[] = { "id", "fanin", NULL };

    borrowed_ref<PyObject> fanin;

    Arg_ParseTupleAndKeywords(args, kwds, "|iO", kwlist, &id, &fanin);

    if( fanin )
    {
        Wire& fi = Wire::ensure(fanin);

        ZZ::Wire po = N.add(ZZ::PO_(id));
        po.set(0, fi.w);

        return Wire::build(po);
    }

    return Wire::build(N.add(ZZ::PO_(id)));
}

ref<PyObject>
Netlist::add_Flop(PyObject* args, PyObject* kwds)
{
    int id = N.typeCount(ZZ::gate_Flop);

    static char *kwlist[] = { "id", "init", NULL };

    borrowed_ref<PyObject> init;

    Arg_ParseTupleAndKeywords(args, kwds, "|iO", kwlist, &id, &init);

    ZZ::lbool init_value = ZZ::l_False;

    if ( init )
    {
        long val = Int_AsLong(init);

        if ( val<0 || val > 3 )
        {
            throw exception(PyExc_ValueError);
        }

        init_value = ZZ::lbool_new(val);
    }

    ZZ::Wire ff = N.add(ZZ::Flop_(id));

    ZZ::Get_Pob(N, flop_init);
    flop_init(ff) = init_value;

    return Wire::build(ff);
}

void
Netlist::add_property(PyObject* o)
{
    Wire& w = Wire::ensure(o);

    ZZ::Get_Pob(N, properties);
    properties.push(w.w);
}

void
Netlist::add_constraint(PyObject* o)
{
    Wire& w = Wire::ensure(o);

    ZZ::Get_Pob(N, constraints);
    constraints.push(w.w);
}

void
Netlist::add_fair_property(PyObject* o)
{
    ZZ::Vec<ZZ::Wire> fcs;
    zzvec_from_iter<Wire>(fcs, o);

    ZZ::Get_Pob(N, fair_properties);
    fair_properties.push();
    fcs.copyTo(fair_properties.last());
}

void
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

    if( sinks.size() <= 0)
    {
        throw exception::format(PyExc_ValueError, "netlist.uporder() needs at least one sink");
    }

    ZZ::Vec<ZZ::gate_id> order;
    ZZ::upOrder(sinks, order, false);

    ZZ::Vec<ZZ::Wire> worder;

    for(uind i=0 ; i<order.size() ; i++)
    {
        worder.push( N[order[i]] );
    }

    return VecIterator<Wire>::build(worder);
}

} // namespace pyzz
