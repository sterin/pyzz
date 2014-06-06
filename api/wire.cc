#include "wire.h"

namespace pyzz
{

using namespace py;

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
        PYTHONWRAPPER_METH_NOARGS(Wire, is_Buf, 0, ""),
        PYTHONWRAPPER_METH_NOARGS(Wire, is_PO, 0, ""),

        PYTHONWRAPPER_METH_O(Wire, implies, 0, ""),
        PYTHONWRAPPER_METH_VARARGS(Wire, ite, 0, ""),
        PYTHONWRAPPER_METH_O(Wire, equals, 0, ""),

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
    Wire& rhs = Wire::ensure(o);

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
Wire::equals(PyObject* o)
{
    Wire& rhs = Wire::ensure(o);
    return build( s_Equiv(w, rhs.w) ) ;
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
Wire::is_Buf()
{
    return Bool_FromLong( type(w) == ZZ::gate_Buf );
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

} // namespace pyzz
