#include "unroll.h"

#include "wire.h"
#include "netlist.h"
#include "veciterator.h"

#include <structmember.h>

namespace pyzz
{

using namespace py;

Unroll::Unroll(borrowed_ref<Netlist> N_, bool init) :
    _init(init),
    _N(N_),
    _F(Netlist::build())
{
    N = Netlist::ensure(_N).N;
    F = Netlist::ensure(_F).N;
}

Unroll::~Unroll()
{
}

void Unroll::construct(Unroll* p, PyObject* args, PyObject* kwds)
{
    static char *kwlist[] = { "N", "init", NULL };

    borrowed_ref<PyObject> pN;
    borrowed_ref<PyObject> pinit = True;

    Arg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &pN, &pinit);

    Netlist::ensure(pN);
    bool init = Object_IsTrue(pinit);

    new (p) Unroll(pN, init);
}

void
Unroll::initialize(PyObject* module)
{
    static PyMappingMethods as_mapping = { 0 };
    as_mapping.mp_subscript = wrappers::binaryfunc<Unroll, &Unroll::mp_subscript>;
    as_mapping.mp_length = wrappers::mp_length<Unroll, &Unroll::mp_length>;
    _type.tp_as_mapping = &as_mapping;

    static PySequenceMethods as_sequence = { 0 };
    as_sequence.sq_contains = wrappers::objobjproc<Unroll, &Unroll::sq_contains>;
    _type.tp_as_sequence = &as_sequence;

    static PyGetSetDef getset[] = {
        PYTHONWRAPPER_GETTER(N, Unroll, _N, "Netlist"),
        PYTHONWRAPPER_GETTER(F, Unroll, _F, "Unrolled netlist"),
        { NULL } // sentinel
    };
    _type.tp_getset = getset;

    base::initialize("_pyzz.unroll");
    add_to_module(module, "unroll");
}

Py_ssize_t
Unroll::mp_length()
{
    return _maps.size();
}

ref<PyObject>
Unroll::mp_subscript(PyObject* args)
{
    int k;
    borrowed_ref<PyObject> o;

    Arg_ParseTuple(args, "Oi", &o, &k);

    try
    {
        ZZ::Vec<ZZ::Wire> vec;

        pywrapper_for_iterator(o, pw)
        {
            Wire& w = Wire::ensure(pw);
            vec.push( unroll(w.w, k) );
        }

        return VecIterator<Wire>::build(vec);
    }
    catch( exception& e )
    {
        exception::handle(PyExc_TypeError);
    }

    Wire& w = Wire::ensure(o);
    return Wire::build( unroll(w.w, k) );
}

int
Unroll::sq_contains(PyObject* args)
{
    unsigned long k;
    borrowed_ref<PyObject> o;

    Arg_ParseTuple(args, "Ok", &o, &k);

    Wire& w = Wire::ensure(o);

    if( k>= _maps.size() )
    {
        return 0;
    }

    ZZ::GLit l = +_maps[k][w.w];

    return l != ZZ::glit_NULL;
}

void
Unroll::ensure_frame(uint k)
{
    uind prev_size = _maps.size();

    _maps.growTo(k + 1);

    for( uind i=prev_size; i<_maps.size(); i++)
    {
        _maps[i](N.True()) = F.True();
    }

    _visited.growTo(k + 1);
}

bool
Unroll::is_visited(stack_elem e)
{
    return _visited[e.k].has(e.lit);
}

bool
Unroll::visit(stack_elem e)
{
    return _visited[e.k].add(e.lit);
}

void
Unroll::push_children(stack_elem e)
{
    ZZ::Wire w = N[e.lit];

    if( ZZ::type(w) == ZZ::gate_And )
    {
        _dfs_stack.push( stack_elem(+w[0], e.k) );
        _dfs_stack.push( stack_elem(+w[1], e.k) );
    }
    else if( ZZ::type(w) == ZZ::gate_Flop && e.k>0 )
    {
        _dfs_stack.push( stack_elem(+w[0], e.k-1) );
    }
    else if( ZZ::type(w) == ZZ::gate_PO )
    {
        _dfs_stack.push( stack_elem(+w[0], e.k) );
    }
}

void
Unroll::unroll_wire(stack_elem e)
{
    ZZ::Wire w = N[+e.lit];
    uint k = e.k;

    ZZ::WWMap& M = _maps[k];

    if( ZZ::type(w) == ZZ::gate_And )
    {
        M(w) = ZZ::s_And( F[M[w[0]]], F[M[w[1]]]);
    }
    else if( ZZ::type(w) == ZZ::gate_PI )
    {
        M(w) = F.add( ZZ::PI_( F.typeCount(ZZ::gate_PI) ) );
    }
    else if( ZZ::type(w) == ZZ::gate_Flop )
    {
        if( k==0 )
        {
            if( _init )
            {
                Get_Pob(N, flop_init);
                ZZ::lbool init_val = flop_init[w];

                if( init_val == ZZ::l_True)
                {
                    M(w) = F.True();
                    return;
                }
                else if ( init_val == ZZ::l_False)
                {
                    M(w) = ~F.True();
                    return;
                }
            }

            M(w) = F.add( ZZ::PI_( F.typeCount(ZZ::gate_PI) ) );
            return;
        }
        else
        {
            M(w) = F[_maps[k-1][w[0]]];
            return;
        }
    }
    else if( ZZ::type(w) == ZZ::gate_PO )
    {
        ZZ::Wire f = F.add( ZZ::PO_( F.typeCount(ZZ::gate_PO) ) );
        f.set(0, F[M[w[0]]] );

        M(w) = f;
        return;
    }
}

ZZ::Wire
Unroll::unroll(ZZ::GLit w, uint k)
{
    ensure_frame(k);

    _dfs_stack.push( stack_elem(+w, k) );

    while( _dfs_stack.size() > 0 )
    {
        stack_elem cur = _dfs_stack.popC();

        if( cur.is_marked() )
        {
            unroll_wire(cur);
        }
        else if( ! is_visited(cur) )
        {
            visit(cur);
            _dfs_stack.push(cur.mark());
            push_children(cur);
        }
    }

    return F[_maps[k][w]];
}

} // namespace pyzz
