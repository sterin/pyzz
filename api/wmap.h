#ifndef pyzz_wmap__H
#define pyzz_wmap__H

#include "pyzzwrapper.h"
#include "wire.h"

namespace pyzz
{

template<typename pytype>
class WMap :
    public type_base_with_new<WMap<pytype>>
{
public:

    typedef typename pytype::zztype zztype;

    typedef WMap<pytype> C;
    typedef ZZ::WMap<zztype> ZC;
    typedef type_base_with_new<C> base;

    WMap()
    {
    }

    ~WMap()
    {
    }

    static void construct(C* p, PyObject* args, PyObject* kwds)
    {
        new (p) C();

        static char *kwlist[] = { "seq", NULL };

        borrowed_ref<PyObject> seq;

        Arg_ParseTupleAndKeywords(args, kwds, "|O", kwlist, &seq);

        if( seq )
        {
            p->update(seq);
        }
    }

    static void initialize(PyObject* module, const char* mname, const char* name)
    {
        static PyMappingMethods as_mapping = { 0 };

        as_mapping.mp_length = wrappers::mp_length<C, &C::mp_length>;
        as_mapping.mp_subscript = wrappers::binaryfunc<C, &C::mp_subscript>;
        as_mapping.mp_ass_subscript = wrappers::mp_ass_subscript<C, &C::mp_ass_subscript>;

        base::_type.tp_as_mapping = &as_mapping;

        static PySequenceMethods as_sequence = { 0 };

        as_sequence.sq_contains = wrappers::objobjproc<C, &C::sq_contains>;

        base::_type.tp_as_sequence = &as_sequence;

        static PyMethodDef methods[] = {
            PYTHONWRAPPER_METH_NOARGS(C, clear, 0, "clear the map"),
            PYTHONWRAPPER_METH_O(C, update, 0, "update map from iterable"),
            { NULL }
        };

        base::_type.tp_methods = methods;

        base::initialize(mname);
        base::add_to_module(module, name);
    }

    void clear()
    {
        wmap.clear();
    }

    Py_ssize_t mp_length()
    {
        return wmap.size();
    }

    ref<PyObject> mp_subscript(PyObject* pkey)
    {
        Wire& key = Wire::ensure(pkey);

        zztype val = wmap[+key.w] ^ key.w.sign();

        if( val.null() )
        {
            throw py::exception(PyExc_KeyError);
        }

        return pytype::build( val );
    }

    void mp_ass_subscript(PyObject* pkey, PyObject* pval)
    {
        Wire& key = Wire::ensure(pkey);
        pytype& val = pytype::ensure(pval);

        wmap( +key.w ) = val.val()^key.w.sign();
    }

    int sq_contains(PyObject* pkey)
    {
        Wire& key = Wire::ensure(pkey);

        zztype val = wmap[key.w];

        return val.null() ? 0 : 1;
    }

    void update(PyObject* seq)
    {
        pywrapper_for_iterator(seq, pyitem)
        {
            ref<PyObject> pair = Sequence_Fast(pyitem, "");

            if( Sequence_Fast_GET_SIZE(pair) != 2)
            {
                throw exception(PyExc_TypeError);
            }

            Wire& key = Wire::ensure(Sequence_Fast_GET_ITEM(pair, 0));
            pytype& val = pytype::ensure(Sequence_Fast_GET_ITEM(pair, 1));

            wmap(+key.w) = key.w ^ val.val()^key.w.sign();
        }
    }

public:

    ZC wmap;
};

} // namesapce pyzz

#endif // #ifndef pyzz_wmap__H
