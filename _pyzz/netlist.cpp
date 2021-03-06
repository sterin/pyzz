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
    typedef ZZ::Pec_FlopInit ZC;
    typedef type_base<C> base;

    FlopInitMap(ZC& m, borrowed_ref<PyObject> p) :
        _wmap(m),
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
        ZZ::lbool val = _wmap[key];
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

        _wmap(key) = ZZ::lbool_new(val);
    }

private:

    ZC& _wmap;
    ref<PyObject> _ref;
};

class NameStore :
    public type_base<NameStore>
{
public:

    typedef NameStore C;
    typedef type_base<NameStore> base;

    NameStore(ZZ::NameStore& ns, borrowed_ref<Netlist> N) :
        _ns(ns),
        _N(N)
    {
    }

    ~NameStore()
    {
    }

    static void initialize(PyObject* module, const char* mname, const char* name)
    {
        static PyMethodDef methods[] = {
            PYTHONWRAPPER_METH_VARARGS( NameStore, add, 0, ""),
            PYTHONWRAPPER_METH_O( NameStore, invert, 0, ""),
            PYTHONWRAPPER_METH_O( NameStore, clear, 0, ""),
            { NULL }  // sentinel
        };

        _type.tp_methods = methods;

        static PyMappingMethods as_mapping = { 0 };

        as_mapping.mp_subscript = wrappers::binaryfunc<C, &C::mp_subscript>;

        base::_type.tp_as_mapping = &as_mapping;

        static PySequenceMethods as_sequence = { 0 };

        as_sequence.sq_contains = wrappers::objobjproc<C, &C::sq_contains>;

        base::_type.tp_as_sequence = &as_sequence;

        base::initialize(mname);
        base::add_to_module(module, name);
    }

    void add(PyObject* args)
    {
        borrowed_ref<PyObject> o;
        char* name;

        Arg_ParseTuple(args, "Os", &o, &name);

        Wire& w = Wire::ensure(o);

        _ns.add(w.w, name);
    }

    void invert(PyObject* o)
    {
        Wire& w = Wire::ensure(o);
        _ns.invert(w.w);
    }

    void clear(PyObject* o)
    {
        Wire& w = Wire::ensure(o);
        _ns.clear(w.w);
    }

    ref<PyObject> mp_subscript(PyObject* o)
    {
        if ( Wire::check(o) )
        {
            Wire& w = Wire::ensure(o);
            const uind size = _ns.size(w.w);

            if( size == 0)
            {
                throw exception(PyExc_KeyError);
            }

            ref<PyObject> list = List_New(size);
            ZZ::Vec<char> name;

            for( uind i=0 ; i<size ; i++)
            {
                _ns.get(w.w, name, i);
                List_SetItem(list, i, String_FromString(name.base()));
            }

            return list;
        }

        const char* name = String_AsString(o);

        ZZ::GLit gl = _ns.lookup(name);

        if ( gl == ZZ::glit_NULL )
        {
            throw exception(PyExc_KeyError);
        }

        return Wire::build(_N->N[gl]);
    }

    int sq_contains(PyObject* o)
    {
        if ( Wire::check(o) )
        {
            Wire& w = Wire::ensure(o);
            return _ns.size(w.w)>0;
        }

        const char* name = String_AsString(o);
        ZZ::GLit gl = _ns.lookup(name);

        return gl != ZZ::glit_NULL;
    }

private:

    ZZ::NameStore& _ns;
    ref<Netlist> _N;
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
    Assure_Pob0(N, strash);
    Assure_Pob0(N, flop_init);
    Assure_Pob0(N, properties);
    Assure_Pob0(N, constraints);
    Assure_Pob0(N, fair_properties);
    Assure_Pob0(N, fair_constraints);

    _PIs.clear();

    For_Gatetype(N, ZZ::gate_PI, pi)
    {
        _PIs.push(pi);
    }

    _POs.clear();

    For_Gatetype(N, ZZ::gate_PO, po)
    {
        _POs.push(po);
    }

    _Flops.clear();

    For_Gatetype(N, ZZ::gate_Flop, ff)
    {
        _Flops.push(ff);
    }

    //N.names().enableLookup();
}

void
Netlist::ensure_netlist(ZZ::Wire w)
{
    if ( w.nl() != N.nl() )
    {
        throw exception(zz_error, "netlist mismatch");
    }
}

void
Netlist::initialize(PyObject* module)
{
    static PyMethodDef methods[] = {

        PYTHONWRAPPER_METH_O( Netlist, unflatten_aiger, METH_STATIC, ""),
        PYTHONWRAPPER_METH_O( Netlist, read_aiger, METH_STATIC, ""),
        PYTHONWRAPPER_METH_O( Netlist, read, METH_STATIC, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, flatten_aiger, 0, ""),
        PYTHONWRAPPER_METH_O( Netlist, write_aiger, 0, ""),
        PYTHONWRAPPER_METH_O( Netlist, write, 0, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, get_True, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, get_False, 0, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, add_PI, 0, ""),
        PYTHONWRAPPER_METH_KEYWORDS( Netlist, add_PO, 0, ""),
        PYTHONWRAPPER_METH_KEYWORDS( Netlist, add_Flop, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, add_Buf, 0, ""),

        PYTHONWRAPPER_METH_O( Netlist, add_property, 0, ""),
        PYTHONWRAPPER_METH_O( Netlist, add_constraint, 0, ""),
        PYTHONWRAPPER_METH_O( Netlist, add_fair_property, 0, ""),
        PYTHONWRAPPER_METH_O( Netlist, add_fair_constraint, 0, ""),

        PYTHONWRAPPER_METH_O( Netlist, get_PI, 0, ""),
        PYTHONWRAPPER_METH_O( Netlist, get_PO, 0, ""),
        PYTHONWRAPPER_METH_O( Netlist, get_Flop, 0, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, get_PIs, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, get_POs, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, get_Flops, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, get_Bufs, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, get_Ands, 0, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, get_properties, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, get_constraints, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, get_fair_properties, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, get_fair_constraints, 0, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, clear_properties, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, clear_constraints, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, clear_fair_properties, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, clear_fair_constraints, 0, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, n_PIs, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, n_POs, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, n_Flops, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, n_Bufs, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, n_Ands, 0, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, n_properties, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, n_constraints, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, n_fair_properties, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, n_fair_constraints, 0, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, copy, 0, ""),

        PYTHONWRAPPER_METH_O( Netlist, uporder, 0, ""),

        PYTHONWRAPPER_METH_NOARGS( Netlist, remove_unreach, 0, ""),
        PYTHONWRAPPER_METH_NOARGS( Netlist, remove_bufs, 0, ""),

        { NULL }  // sentinel
    };

    _type.tp_methods = methods;

    static PyGetSetDef getset[] = {
        PYTHONWRAPPER_GETTER(flop_init, Netlist, get_flop_init, ""),
        PYTHONWRAPPER_GETTER(names, Netlist, get_names, ""),
        { NULL } // sentinel
    };

    _type.tp_getset = getset;


    base::initialize("pyzz.netlist");
    add_to_module(module, "netlist");

    FlopInitMap::initialize(module, "_pyzz.flop_init_map", "flop_init_map");
    NameStore::initialize(module, "_pyzz.name_store", "name_store");
}

ref<PyObject>
Netlist::unflatten_aiger(PyObject* o)
{
    if( ! Object_CheckBuffer(o) )
    {
        throw exception( PyExc_TypeError, "expected an object that supports the buffer interface" );
    }

    ref<PyObject> mv = MemoryView_GetContiguous(o, PyBUF_READ, 'C');
    Py_buffer* buf = MemoryView_GET_BUFFER(mv);

    ZZ::In in{reinterpret_cast<cchar*>(buf->buf), static_cast<uind>(buf->len), false};
    ref<Netlist> NN = Netlist::build(true);
    
    ZZ::readAiger(in, NN->N);
    NN->assure_pobs();
    
    return NN;
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

ref<PyObject>
Netlist::flatten_aiger()
{
    ZZ::Out out;
    writeAiger(out, N, ZZ::Array<uchar>());

    ZZ::Vec<char>& data = out.vec();
    return ByteArray_FromStringAndSize(data.base(), data.size());
}

void
Netlist::write_aiger(PyObject* o)
{
    writeAigerFile( String_AsString(o) , N, ZZ::Array<uchar>(), true);
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
Netlist::get_False()
{
    return Wire::build(~N.True());
}

ref<PyObject>
Netlist::add_PI()
{
    ZZ::Wire pi = N.add(ZZ::PI_(N.typeCount(ZZ::gate_PI)));
    _PIs.push(pi);

    return Wire::build(pi);
}

ref<PyObject>
Netlist::add_PO(PyObject* args, PyObject* kwds)
{
    const int id = N.typeCount(ZZ::gate_PO);

    static char *kwlist[] = { "fanin", NULL };

    borrowed_ref<PyObject> fanin;

    Arg_ParseTupleAndKeywords(args, kwds, "|O", kwlist, &fanin);

    if( fanin )
    {
        Wire& fi = Wire::ensure(fanin);
        ensure_netlist(fi.w);

        ZZ::Wire po = N.add(ZZ::PO_(id));
        _POs.push(po);

        po.set(0, fi.w);

        return Wire::build(po);
    }

    ZZ::Wire po = N.add(ZZ::PO_(id));
    _POs.push(po);

    return Wire::build(po);
}

ref<PyObject>
Netlist::add_Flop(PyObject* args, PyObject* kwds)
{
    int id = N.typeCount(ZZ::gate_Flop);

    static char *kwlist[] = { "init", "next", NULL };

    borrowed_ref<PyObject> init;
    borrowed_ref<PyObject> next;

    Arg_ParseTupleAndKeywords(args, kwds, "|OO", kwlist, &init, &next);

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


    Wire* nw = nullptr;

    if (next)
    {
        nw = &Wire::ensure(next);
        ensure_netlist(nw->w);
    }

    ZZ::Wire ff = N.add(ZZ::Flop_(id));
    _Flops.push(ff);

    Get_Pob(N, flop_init);
    flop_init(ff) = init_value;

    if ( next )
    {
        ff.set(0, nw->w);
    }

    return Wire::build(ff);
}

ref<PyObject>
Netlist::add_Buf()
{
    return Wire::build(N.add(ZZ::Buf_()));
}

void
Netlist::add_property(PyObject* o)
{
    Wire& w = Wire::ensure(o);
    ensure_netlist(w.w);

    Get_Pob(N, properties);
    properties.push(w.w);
}

void
Netlist::add_constraint(PyObject* o)
{
    Wire& w = Wire::ensure(o);
    ensure_netlist(w.w);

    Get_Pob(N, constraints);
    constraints.push(w.w);
}

void
Netlist::add_fair_property(PyObject* o)
{
    ZZ::Vec<ZZ::Wire> fcs;
    zzvec_from_iter<Wire>(fcs, o);

    for(uind i=0 ; i<fcs.size() ; i++)
    {
        ensure_netlist(fcs[i]);
    }

    Get_Pob(N, fair_properties);
    fair_properties.push();
    fcs.copyTo(fair_properties.last());
}

void
Netlist::add_fair_constraint(PyObject* o)
{
    Wire& w = Wire::ensure(o);
    ensure_netlist(w.w);

    Get_Pob(N, fair_constraints);
    fair_constraints.push(w.w);
}

void
Netlist::remove_unreach()
{
    Remove_Pob(N, strash);
    removeUnreach(N);

    assure_pobs();
}

void
Netlist::remove_bufs()
{
    removeBuffers(N);
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
Netlist::n_Bufs()
{
    return Int_FromSize_t(N.typeCount(ZZ::gate_Buf));
}

ref<PyObject>
Netlist::n_Ands()
{
    return Int_FromSize_t(N.typeCount(ZZ::gate_And));
}


ref<PyObject>
Netlist::n_properties()
{
    Get_Pob(N, properties);
    return Int_FromLong( properties.size() );
}

ref<PyObject>
Netlist::n_constraints()
{
    Get_Pob(N, constraints);
    return Int_FromLong( constraints.size() );
}

ref<PyObject>
Netlist::n_fair_properties()
{
    Get_Pob(N, fair_properties);
    return Int_FromLong( fair_properties.size() );
}

ref<PyObject>
Netlist::n_fair_constraints()
{
    Get_Pob(N, fair_constraints);
    return Int_FromLong( fair_constraints.size() );
}

ref<PyObject>
Netlist::get_PI(PyObject* o)
{
    uind i = Int_AsSsize_t(o);

    if( i>=_PIs.size() )
    {
        throw exception(PyExc_KeyError);
    }

    return Wire::build(_PIs[i]);
}

ref<PyObject>
Netlist::get_PO(PyObject* o)
{
    uind i = Int_AsSsize_t(o);

    if( i>=_POs.size() )
    {
        throw exception(PyExc_KeyError);
    }

    return Wire::build(_POs[i]);
}

ref<PyObject>
Netlist::get_Flop(PyObject* o)
{
    uind i = Int_AsSsize_t(o);

    if( i>=_Flops.size() )
    {
        throw exception(PyExc_KeyError);
    }

    return Wire::build(_Flops[i]);
}

ref<PyObject>
Netlist::get_PIs()
{
    return VecRef<Wire>::build(borrow(this), _PIs);
}

ref<PyObject>
Netlist::get_POs()
{
    return VecRef<Wire>::build(borrow(this), _POs);
}

ref<PyObject>
Netlist::get_Flops()
{
    return VecRef<Wire>::build(borrow(this), _Flops);
}

ref<PyObject>
Netlist::get_Bufs()
{
    ZZ::Vec<ZZ::Wire> vec;

    fill_gates(vec, N, ZZ::gate_Buf);

    return Vec<Wire>::build(vec);
}

ref<PyObject>
Netlist::get_Ands()
{
    ZZ::Vec<ZZ::Wire> vec;

    fill_gates(vec, N, ZZ::gate_And);

    return Vec<Wire>::build(vec);
}

ref<PyObject>
Netlist::get_properties()
{
    Get_Pob(N, properties);

    ZZ::Vec<ZZ::Wire> props;
    properties.copyTo(props);

    return Vec<Wire>::build(props);
}


ref<PyObject>
Netlist::get_constraints()
{
    Get_Pob(N, constraints);

    ZZ::Vec<ZZ::Wire> props;
    constraints.copyTo(props);

    return Vec<Wire>::build(props);
}

ref<PyObject>
Netlist::get_fair_properties()
{
    Get_Pob(N, fair_properties);

    ref<PyObject> list=List_New(fair_properties.size());

    ZZ::Vec<ZZ::Wire> props;

    for( uind i=0; i<fair_properties.size() ; i++)
    {
        fair_properties[i].copyTo(props);
        List_SetItem(list, i, Vec<Wire>::build(props));
    }

    return list;
}

ref<PyObject>
Netlist::get_fair_constraints()
{
    Get_Pob(N, fair_constraints);

    ZZ::Vec<ZZ::Wire> props;
    fair_constraints.copyTo(props);

    return Vec<Wire>::build(props);
}

void
Netlist::clear_properties()
{
    Get_Pob(N, properties);
    properties.clear();
}


void
Netlist::clear_constraints()
{
    Get_Pob(N, constraints);
    constraints.clear();
}

void
Netlist::clear_fair_properties()
{
    Get_Pob(N, fair_properties);
    fair_properties.clear();
}

void
Netlist::clear_fair_constraints()
{
    Get_Pob(N, fair_constraints);
    fair_constraints.clear();
}

ref<PyObject>
Netlist::get_flop_init()
{
    Get_Pob(N, flop_init);
    borrowed_ref<Netlist> pp(this);
    return FlopInitMap::build(flop_init, pp);
}

ref<PyObject>
Netlist::get_names()
{
    borrowed_ref<Netlist> pp(this);
    return NameStore::build(N.names(), pp);
}

void
Netlist::copy_props(ZZ::Netlist& M, const ZZ::WWMap& xlat)
{
    Get_Pob(N, properties);
    Get_Pob2(M, properties, M_properties);

    for(uind i=0; i<properties.size() ; i++)
    {
        ZZ::Wire w = properties[i];
        M_properties.push( M[xlat[w]] );
    }

    Get_Pob(N, constraints);
    Get_Pob2(M, constraints, M_constraints);

    for(uind i=0; i<constraints.size() ; i++)
    {
        ZZ::Wire w = constraints[i];
        M_constraints.push( M[xlat[w]]);
    }

    Get_Pob(N, fair_properties);
    Get_Pob2(M, fair_properties, M_fair_properties);

    for(uind i=0; i<fair_properties.size() ; i++)
    {
        ZZ::Vec<ZZ::Wire>& v = fair_properties[i];

        M_fair_properties.push();
        ZZ::Vec<ZZ::Wire>& mv = M_fair_properties.last();
        mv.setSize(v.size());

        for(uind j=0 ; j<v.size() ; j++)
        {
            ZZ::Wire w = v[j];
            mv[j] = M[xlat[w]];
        }
    }

    Get_Pob(N, fair_constraints);
    Get_Pob2(M, fair_constraints, M_fair_constraints);

    for(uind i=0; i<fair_constraints.size() ; i++)
    {
        ZZ::Wire w = fair_constraints[i];
        M_fair_constraints.push( M[xlat[w]]);
    }
}

void
Netlist::copy_names(ZZ::Netlist& M, const ZZ::WWMap& xlat)
{
    ZZ::NameStore& names = N.names();
    ZZ::NameStore& M_names = M.names();

    ZZ::Vec<char> name;

    For_Gates(N, w)
    {
        ZZ::Wire mw = M[xlat[w]];

        for(uind i=0; i<names.size(w); i++ )
        {
            names.get(w, name, i);
            M_names.add(mw, name.base());
        }
    }
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

    Get_Pob(N, flop_init);
    Get_Pob2(M, flop_init, M_flop_init);

    For_Gatetype(N, ZZ::gate_Flop, ff)
    {
        int id = M.typeCount(ZZ::gate_Flop);
        ZZ::Wire mff = M.add(ZZ::Flop_(id));
        xlat(ff) = mff;
        M_flop_init(mff) = flop_init[ff];
    }

    ZZ::Vec<ZZ::gate_id> and_order;

    For_Gatetype(N, ZZ::gate_And, g)
    {
        and_order.push(id(g));
    }

    upOrder(N, and_order, false, false);

    for(uind i=0 ; i<and_order.size() ; i++)
    {
        ZZ::Wire g = N[and_order[i]];

        if ( type(g) == ZZ::gate_And )
        {
            ZZ::Wire g0 = M[xlat[g[0]]];
            ZZ::Wire g1 = M[xlat[g[1]]];

            xlat(g) = ZZ::s_And(g0, g1);
        }
        else if( type(g) == ZZ::gate_Buf )
        {
            ZZ::Wire g0 = M[xlat[g[0]]];
            xlat(g) = M.add( ZZ::Buf_(), g0);
        }
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
        xlat(po) = mpo;
        mpo.set(0, xlat[po[0]]);
    }

    copy_props(M, xlat);
    copy_names(M, xlat);

    ref<WMap<Wire>> pyxlat = WMap<Wire>::build();

    pyxlat->wmap(N.True()) = M.True();

    For_Gates(N, w)
    {
        pyxlat->wmap(w) = M[xlat[w]];
    }

    CN->assure_pobs();

    ref<PyObject> tuple = Tuple_New(2);

    Tuple_SetItem(tuple, 0, CN);
    Tuple_SetItem(tuple, 1, pyxlat);

    return tuple;
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

    return Vec<Wire>::build(worder);
}

} // namespace pyzz
