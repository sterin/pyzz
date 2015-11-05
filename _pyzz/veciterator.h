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

    VecIterator(borrowed_ref<PyObject> o, const vec_type& vec) :
        _o(o),
        _vec(vec),
        _i(0)
    {
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

    ref<PyObject> _o;
    const vec_type& _vec;
    uind _i;
};

template<typename user_type, typename pytype>
class VecBase :
    public type_base<user_type>
{
public:

    typedef typename pytype::zztype zztype;

    typedef VecBase<user_type,pytype> C;
    typedef type_base<user_type> base;

    typedef ZZ::Vec<zztype> vec_type;

    VecBase(const vec_type& vec) :
        _vec(vec)
    {
    }

    ~VecBase()
    {
    }

    static void initialize(PyObject* module, const char* mname, const char* name)
    {
        base::_type.tp_iter = wrappers::unaryfunc<C,&C::tp_iter>;

        static PyMappingMethods as_mapping = { 0 };

        as_mapping.mp_subscript = wrappers::binaryfunc<C, &C::mp_subscript>;
        as_mapping.mp_length = wrappers::mp_length<C, &C::mp_length>;

        base::_type.tp_as_mapping = &as_mapping;

        base::initialize(mname);
        base::add_to_module(module, name);
    }

    ref<PyObject> tp_iter()
    {
        return VecIterator<pytype>::build(borrow(this), _vec);
    }

    ref<PyObject> mp_subscript(PyObject* o)
    {
        uind i = Int_AsLong(o);

        if( i >= _vec.size() )
        {
            throw exception(PyExc_KeyError);
        }

        return pytype::build( _vec[i] );
    }

    Py_ssize_t mp_length()
    {
        return _vec.size();
    }

protected:

    const vec_type& _vec;
};

template<typename pytype>
class VecRef :
    public VecBase<VecRef<pytype>, pytype>
{
public:
    typedef typename pytype::zztype zztype;
    typedef ZZ::Vec<zztype> vec_type;

    VecRef(borrowed_ref<PyObject> o, const vec_type& vec) :
        VecBase<VecRef<pytype>, pytype>(vec),
        _o(o)
    {

    }

private:
    ref<PyObject> _o;
};

template<typename pytype>
class Vec :
    public VecBase<Vec<pytype>, pytype>
{
public:
    typedef typename pytype::zztype zztype;
    typedef ZZ::Vec<zztype> vec_type;

    Vec(vec_type& vec) :
        VecBase<Vec<pytype>, pytype>(_realvec)
    {
        vec.moveTo(_realvec);
    }

private:
    vec_type _realvec;
};

} // namesapce pyzz

#endif // #ifndef pyzz_veciterator__H
