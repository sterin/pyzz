#ifndef pyzz_unroll__H
#define pyzz_unroll__H

#include "pyzzwrapper.h"

namespace pyzz
{

class Unroll :
    public type_base_with_new<Unroll>
{
public:

    Unroll();
    ~Unroll();

    static void initialize(PyObject* module);

    void tp_init(PyObject* args, PyObject* kwds);

    ref<PyObject> unroll(PyObject* o);

    ref<PyObject> mp_subscript(PyObject* o);

private:

    void unroll_frame();
    ZZ::Wire unroll_wire(ZZ::Wire w, int k);

    ref<PyObject> _N;
    ref<PyObject> _F;

    ZZ::NetlistRef N;
    ZZ::NetlistRef F;

    bool _init;
    int _next;
    ZZ::Vec<ZZ::WWMap> _maps;
};

} // namesapce pyzz

#endif // #ifndef pyzz_unroll__H
