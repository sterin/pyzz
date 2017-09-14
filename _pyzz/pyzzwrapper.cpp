#include "pyzzwrapper.h"

#include "lit.h"
#include "wire.h"
#include "wmap.h"
#include "veciterator.h"
#include "netlist.h"
#include "solver.h"
#include "unroll.h"

#include "readonly_static_descriptor.h"

#include "ZZ_Bip.hh"

#include <vector>

extern "C" 
unsigned Abc_TtCanonicize(void* pTruth, int nVars, char* pCanonPerm);

namespace pyzz
{

PyObject* parse_error;
PyObject* aiger_parse_error;
PyObject* zz_error;

using namespace py;

ref<PyObject>
imc(PyObject* args, PyObject* kwds)
{
    static char *kwlist[] = { "N", "props", "first_k", "quiet", NULL };

    borrowed_ref<PyObject> pN;
    borrowed_ref<PyObject> pyprops;
    int first_k = 0;
    int quiet = 1;
    Arg_ParseTupleAndKeywords(args, kwds, "O|Oii:imc", kwlist, &pN, &pyprops, &first_k, &quiet);

    if( first_k < 0)
    {
        first_k = 0;
    }

    Netlist& N = Netlist::ensure(pN);

    ZZ::Vec<ZZ::Wire> props;

    if( pyprops )
    {
        pywrapper_for_iterator(pyprops, pyitem)
        {
            Wire& ww = Wire::ensure(pyitem);
            props.push(ww.w);
        }
    }
    else
    {
        Get_Pob(N.N, properties);
        properties.copyTo(props);
    }

    int buf_free_depth = -1;

    ZZ::Params_ImcStd params;

    params.first_k = first_k;
    params.quiet = quiet;

    ZZ::lbool res = ZZ::imcStd(
        N.N,
        props,
        params,
        0,
        ZZ::NetlistRef(),
        &buf_free_depth,
        0
        );

    return BuildValue("ii", res.value, buf_free_depth);
}

ref<PyObject>
pdr(PyObject* args, PyObject* kwds)
{
    static char *kwlist[] = { "N", "props", "first_k", "quiet", NULL };

    borrowed_ref<PyObject> pN;
    borrowed_ref<PyObject> pyprops;
    int first_k = 0;
    int quiet = 1;
    Arg_ParseTupleAndKeywords(args, kwds, "O|Oii:imc", kwlist, &pN, &pyprops, &first_k, &quiet);

    if( first_k < 0)
    {
        first_k = 0;
    }

    Netlist& N = Netlist::ensure(pN);

    ZZ::Vec<ZZ::Wire> props;

    if( pyprops )
    {
        pywrapper_for_iterator(pyprops, pyitem)
        {
            Wire& ww = Wire::ensure(pyitem);
            props.push(ww.w);
        }
    }
    else
    {
        Get_Pob(N.N, properties);
        properties.copyTo(props);
    }

    int buf_free_depth = -1;

    ZZ::Params_Pdr params;

    //params.minimal_cex = first_k;
    params.quiet = quiet;

    ZZ::lbool res = ZZ::propDrivenReach(
        N.N,
        props,
        params,
        0,
        ZZ::NetlistRef(),
        &buf_free_depth,
        0
        );

    return BuildValue("ii", res.value, buf_free_depth);
}

template<typename T, typename F>
ref<PyObject> to_list(const std::vector<T>& vec, F f)
{
    auto list = List_New(vec.size());

    for(int i=0; i<vec.size(); i++)
    {
        List_SetItem(list, i, f(vec[i]));
    }

    return list;
}


ref<PyObject>
abc_tt_canonize(PyObject* args, PyObject* kwargs)
{
    static char *kwlist[] = { "n", "d", NULL };

    int N=0;
    borrowed_ref<PyObject> pD;

    Arg_ParseTupleAndKeywords(args, kwargs, "iO:abc_tt_canonize", kwlist, &N, &pD);

    std::vector<std::uint32_t> words;

    pywrapper_for_iterator(pD, pW)
    {
        words.push_back(Int_AsSsize_t(pW));
    }

    std::vector<char> perm(N);
    unsigned mask = Abc_TtCanonicize(&words[0], N, &perm[0]);

    auto tt = to_list(words, Int_FromLong);
    auto pp = to_list(perm, Int_FromLong);

    return BuildValue("iOO", mask, tt.get(), pp.get());
}

ref<PyObject>
marshal_netlist(PyObject* o)
{
    Netlist& N = Netlist::ensure(o);
    
    ZZ::Vec<uchar> data;
    ZZ::streamOut_Netlist(data, N.N);

    return ByteArray_FromStringAndSize(reinterpret_cast<const char*>(data.base()), data.size());
}

ref<PyObject>
unmarshal_netlist(PyObject* o)
{
    if( ! ByteArray_Check(o) )
    {
        throw exception( PyExc_TypeError );        
    }

    const uchar* p = reinterpret_cast<const uchar*>( ByteArray_AsString(o) );

    ref<Netlist> N = Netlist::build(true);
    ZZ::streamIn_Netlist(p, p+ByteArray_Size(o), N->N);
    N->assure_pobs();

    return N;
}

void
init()
{
    using namespace py;
    using namespace pyzz;

    ref<PyObject> zz_error = Err_NewException("_pyzz.zz_error", PyExc_Exception, 0);
    ref<PyObject> aiger_parse_error = Err_NewException("_pyzz.aiger_parse_error", zz_error, 0);
    ref<PyObject> parse_error = Err_NewException("_pyzz.parse_error", zz_error, 0);

    pyzz::zz_error = zz_error;
    pyzz::aiger_parse_error = aiger_parse_error;
    pyzz::parse_error = parse_error;

    static PyMethodDef pyzz_methods[] = {
        PYTHONWRAPPER_FUNC_KEYWORDS(imc, 0, "interpolation-based model-checking"),
        PYTHONWRAPPER_FUNC_KEYWORDS(pdr, 0, "property-directed-reachability"),
        PYTHONWRAPPER_FUNC_KEYWORDS(abc_tt_canonize, 0, "canonize truth table"),
        PYTHONWRAPPER_FUNC_O(marshal_netlist, 0, "marshal netlist into a bytearray"),
        PYTHONWRAPPER_FUNC_O(unmarshal_netlist, 0, "unmarshal netlist from a bytearray"),
        { 0 }
    };

    borrowed_ref<PyObject> mod = InitModule3(
        "_pyzz",
        pyzz_methods,
        "Python interface to ZZ and Bip"
        );

    Module_AddObject( mod, "zz_error", zz_error );
    Module_AddObject( mod, "aiger_parse_error", aiger_parse_error );
    Module_AddObject( mod, "parse_error", parse_error );

    readonly_static_descriptor::initialize();

    Lit::initialize(mod);
    Wire::initialize(mod);
    WMap<Wire>::initialize(mod,"_pyzz.wwmap", "wwmap");
    WMap<Lit>::initialize(mod,"_pyzz.wlmap", "wlmap");
    VecIterator<Wire>::initialize(mod, "_pyzz.witerator", "witerator");
    Vec<Wire>::initialize(mod, "_pyzz.wvec", "wvec");
    VecRef<Wire>::initialize(mod, "_pyzz.wvecref", "wvecref");
    VecIterator<lbool_proxy>::initialize(mod, "_pyzz.witerator", "witerator");
    Vec<lbool_proxy>::initialize(mod, "_pyzz.lboolvec", "lboolvec");
    VecRef<lbool_proxy>::initialize(mod, "_pyzz.lboolvecref", "lboolvecref");
    Netlist::initialize(mod);
    Solver::initialize(mod);
    Unroll::initialize(mod);
}

void zz_init()
{
    ZZ::zzInitialize();
    init();
}

} // namespace pyzz
