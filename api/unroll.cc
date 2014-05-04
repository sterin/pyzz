#include "unroll.h"

#include "wire.h"
#include "netlist.h"
#include "veciterator.h"

#include <structmember.h>

namespace pyzz
{

using namespace py;

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

} // namespace pyzz
