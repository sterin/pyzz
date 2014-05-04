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

private:

    uind _i;
    vec_type _vec;
};

} // namesapce pyzz

#endif // #ifndef pyzz_veciterator__H
