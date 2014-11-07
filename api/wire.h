#ifndef pyzz_wire__H
#define pyzz_wire__H

#include "pyzzwrapper.h"

namespace pyzz
{

class Wire :
    public type_base<Wire>
{
public:

    typedef ZZ::Wire zztype;

    Wire(ZZ::Wire w);
    ~Wire();

    static void initialize(PyObject* module);

    ref<PyObject> tp_repr();

    int tp_compare(PyObject* rhs);

    bool nb_nonzero();

    ref<PyObject> nb_invert();
    ref<PyObject> nb_positive();

    ref<PyObject> nb_and(PyObject* o);
    ref<PyObject> nb_or(PyObject* o);
    ref<PyObject> nb_xor(PyObject* o);

    ref<PyObject> implies(PyObject* o);
    ref<PyObject> ite(PyObject* o);
    ref<PyObject> equals(PyObject* o);

    ref<PyObject> is_True();
    ref<PyObject> is_PI();
    ref<PyObject> is_And();
    ref<PyObject> is_Flop();
    ref<PyObject> is_Buf();
    ref<PyObject> is_PO();

    ref<PyObject> id();
    ref<PyObject> sign();

    ZZ::Wire& val();

    void mp_ass_subscript(PyObject* key, PyObject* val);
    ref<PyObject> mp_subscript(PyObject* o);

    void remove();

    long tp_hash();

public:

    ZZ::Wire w;

private:
    void ensure_netlist(ZZ::Wire w);
};

} // namesapce pyzz

#endif // #ifndef pyzz_wire__H
