#ifndef pyzz_wrapper__H
#define pyzz_wrapper__H

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

struct lbool_proxy
{
    typedef ZZ::lbool zztype;

    static ref<PyObject> build(const ZZ::lbool& v)
    {
        return Int_FromLong(v.value);
    }
};

inline int
py_to_lbool(borrowed_ref<PyObject> o)
{
    if( Int_Check(o) )
    {
        long l = Int_AsLong(o);

        if( l!=0 )
        {
            return 1;
        }

        return 0;
    }

    return -1;
}

inline void
fill_gates(ZZ::Vec<ZZ::Wire>& vec, ZZ::NetlistRef N, ZZ::GateType type)
{
    For_Gatetype(N, type, w)
    {
        vec.push(w);
    }
}

void init();

} // namespace pyzz

#endif // pyzz_wrapper__H
