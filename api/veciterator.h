#ifndef pyzz_veciterator__H
#define pyzz_veciterator__H

#include "pyzzwrapper.h"

namespace pyzz
{

template<typename pytype>
class VecIterator :
    public type_base<VecIterator<pytype> >
{
public:

    typedef typename pytype::zztype zztype;

    typedef VecIterator<pytype> C;
    typedef type_base<C> base;

    typedef ZZ::Vec<zztype> vec_type;

    VecIterator(vec_type& vec) :
        _i(0)
    {
        vec.moveTo(_vec);
    }

    ~VecIterator()
    {
    }

    static void initialize(PyObject* module, const char* mname, const char* name)
    {
        base::_type.tp_iter = wrappers::unaryfunc<C,&C::tp_iter>;
        base::_type.tp_iternext = wrappers::unaryfunc<C,&C::tp_iternext>;

        static PyMappingMethods as_mapping = { 0 };

        as_mapping.mp_subscript = wrappers::binaryfunc<C, &C::mp_subscript>;
        as_mapping.mp_length = wrappers::mp_length<C, &C::mp_length>;

        base::_type.tp_as_mapping = &as_mapping;

        base::initialize(mname);
        base::add_to_module(module, name);
    }

    ref<PyObject> tp_iter()
    {
        return borrow(this);
    }

    ref<PyObject> tp_iternext()
    {
        if ( _i >= _vec.size() )
        {
            return ref<PyObject>();
        }

        return pytype::build( _vec[_i++] );
    }

    ref<PyObject> mp_subscript(PyObject* o)
    {
        uind i = Int_AsLong(o);

        if( _i >= _vec.size() )
        {
            throw exception(PyExc_KeyError);
        }

        return pytype::build( _vec[i] );
    }

    Py_ssize_t mp_length()
    {
        return _vec.size();
    }

private:

    uind _i;
    vec_type _vec;
};

} // namesapce pyzz

#endif // #ifndef pyzz_veciterator__H
