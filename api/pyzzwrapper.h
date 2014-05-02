#ifndef zz_wrapper__H
#define zz_wrapper__H

#include "Prelude.hh"
#include "ZZ_Netlist.hh"
#include "ZZ_Bip.Common.hh"

#include "pywrapper/src/pywrapper.h"

namespace pyzz
{

extern PyObject* parse_error;
extern PyObject* aiger_parse_error;
extern PyObject* zz_error;

} // namespace pyzz

#define PYTHONWRAPPER_USER_CATCH \
    \
    catch (const ZZ::Excp_AigerParseError& e) \
    { \
        PyErr_SetString(pyzz::aiger_parse_error, e.msg.c_str()); \
    } \
    \
    catch (const ZZ::Excp_NlParseError& e) \
    { \
        PyErr_SetString(pyzz::parse_error, e.msg.c_str()); \
    } \
    \
    catch (const ZZ::Excp_Msg& e) \
    { \
        PyErr_SetString(pyzz::zz_error, e.msg.c_str()); \
    }

#include "pywrapper/src/pywrapper_types.h"

namespace pyzz
{

using namespace py;

template<typename pytype>
inline void zzvec_from_iter(ZZ::Vec<typename pytype::zztype>& vec, PyObject* seq)
{
    pywrapper_for_iterator(seq, pyitem)
    {
        pytype& item = pytype::ensure(pyitem);
        vec.push(item.w);
    }
}

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

    ref<PyObject> is_True();
    ref<PyObject> is_PI();
    ref<PyObject> is_And();
    ref<PyObject> is_Flop();
    ref<PyObject> is_PO();

    ref<PyObject> id();
    ref<PyObject> sign();

    ZZ::Wire& val();

    void mp_ass_subscript(PyObject* key, PyObject* val);
    ref<PyObject> mp_subscript(PyObject* o);
    
    long tp_hash();

public:

    ZZ::Wire w;
};

template<typename pytype>
class WMap :
    public type_base_with_new<WMap<pytype> >
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

class Netlist :
    public type_base_with_new<Netlist>
{
public:

    Netlist(bool strash=true);
    ~Netlist();

    static void initialize(PyObject* module);

    static ref<PyObject> read_aiger(PyObject* o);
    static ref<PyObject> read(PyObject* o);

    void write_aiger(PyObject* args);
    void write(PyObject* args);

    ref<PyObject> get_True();

    ref<PyObject> add_PI(PyObject* args);
    ref<PyObject> add_PO(PyObject* args);
    ref<PyObject> add_Flop(PyObject* args);

    ref<PyObject> n_PIs();
    ref<PyObject> n_POs();
    ref<PyObject> n_Flops();
    ref<PyObject> n_Ands();

    ref<PyObject> get_PIs();
    ref<PyObject> get_POs();
    ref<PyObject> get_Flops();
    ref<PyObject> get_Ands();

    ref<PyObject> copy();

    ref<PyObject> uporder(PyObject* args);

    void remove_unreach();

public:

    ZZ::Netlist N;
};

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

void init();

} // namespace pyzz

#endif // zz_wrapper__H
