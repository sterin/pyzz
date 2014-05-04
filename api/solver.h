#ifndef pyzz_solver__H
#define pyzz_solver__H

#include "pyzzwrapper.h"

namespace pyzz
{

class Solver :
    public type_base_with_new<Solver>
{
public:

    Solver(ZZ::NetlistRef N);
    ~Solver();

    static PyObject* base_construct(PyTypeObject *subtype, PyObject *args, PyObject *kwds);
    static void construct(Solver* p, PyObject* args, PyObject* kwds);

    static void initialize(PyObject* module);

    ref<PyObject> mp_subscript(PyObject* pkey);
    int sq_contains(PyObject* pkey);

    void keep(PyObject* args);
    void clausify(PyObject* args);

    void clause(PyObject* args, PyObject* kwds);
    void cube(PyObject* args, PyObject* kwds);
    void implication(PyObject* args, PyObject* kwds);
    void equivalence(PyObject* args, PyObject* kwds);

    ref<PyObject> new_var();

    ref<PyObject> solve(PyObject* args);
    ref<PyObject> value(PyObject* args);

    ref<PyObject> apply_cex(PyObject* args, PyObject* kwds);

    ref<PyObject> has_wire(PyObject* o);

    void set_timeout(borrowed_ref<PyObject> timeout);
    ref<PyObject> get_timeout();

    void set_callback(borrowed_ref<PyObject> cb);
    ref<PyObject> get_callback();

private:

    ZZ::Lit get_Lit(PyObject* o);

    bool sat_callback(uint64 work);
    static bool sat_callback(uint64 work, void* data);

    ref<PyObject> _callback;
    bool _fail;

    ref<PyObject> _conflict;

    ZZ::SatStd _S;
    ZZ::NetlistRef _N;
    ZZ::WMap<ZZ::Lit> _wtos;
    ZZ::WZet _keep;
    ZZ::Clausify<ZZ::SatStd> _C;
};

} // namesapce pyzz

#endif // #ifndef pyzz_solver__H
