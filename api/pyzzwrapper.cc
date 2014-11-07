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
        ZZ::Vec<ZZ::Wire> pos;
        fill_gates(pos, N.N, ZZ::gate_PO);

        for(uind i=0; i<pos.size() ; i++)
        {
            props.push( ~pos[i] );
        }
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
        ZZ::Vec<ZZ::Wire> pos;
        fill_gates(pos, N.N, ZZ::gate_PO);

        for(uind i=0; i<pos.size() ; i++)
        {
            props.push( ~pos[i] );
        }
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
        { 0 }
    };

    borrowed_ref<PyObject> mod = InitModule3(
        "pyzz._pyzz",
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

} // namespace pyzz
