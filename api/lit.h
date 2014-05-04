#ifndef pyzz_lit__H
#define pyzz_lit__H

#include "pyzzwrapper.h"

namespace pyzz
{

class Lit :
    public type_base<Lit>
{
public:

    typedef ZZ::Lit zztype;

    Lit(ZZ::Lit w);
    ~Lit();

    static void initialize(PyObject* module);

    ref<PyObject> tp_repr();

    int tp_compare(PyObject* rhs);

    bool nb_nonzero();

    ref<PyObject> nb_invert();
    ref<PyObject> nb_positive();

    ref<PyObject> id();
    ref<PyObject> sign();

    ZZ::Lit& val();

public:

    ZZ::Lit l;
};

} // namesapce pyzz

#endif // #ifndef pyzz_lit__H
