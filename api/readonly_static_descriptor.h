#ifndef pyzz_readonly_static_descriptor__H
#define pyzz_readonly_static_descriptor__H

#include "pyzzwrapper.h"

namespace pyzz
{

class readonly_static_descriptor :
    public type_base<readonly_static_descriptor>
{
public:

    readonly_static_descriptor(borrowed_ref<PyObject> p) :
        _p(p)
    {
    }

    ~readonly_static_descriptor()
    {
    }

    static void initialize()
    {
        _type.tp_descr_get = wrappers::descrgetfunc<readonly_static_descriptor, PyObject>;

        base::initialize("_pyzz.readonly_descriptor");
    }

    ref<PyObject> tp_descr_get(PyObject* inst)
    {
        return _p;
    }

public:

    ref<PyObject> _p;
};

} // namesapce pyzz

#endif // #ifndef pyzz_readonly_static_descriptor__H
