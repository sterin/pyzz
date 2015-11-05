#include "lit.h"

namespace pyzz
{

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

} // namespace pyzz
