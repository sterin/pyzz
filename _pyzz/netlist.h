#ifndef pyzz_netlist__H
#define pyzz_netlist__H

#include "pyzzwrapper.h"

namespace pyzz
{

class Netlist :
    public type_base_with_new<Netlist>
{
public:

    Netlist(bool empty=false);
    ~Netlist();

    static void initialize(PyObject* module);

    static ref<PyObject> read_aiger(PyObject* o);
    static ref<PyObject> read(PyObject* o);

    void write_aiger(PyObject* args);
    void write(PyObject* args);

    ref<PyObject> get_True();
    ref<PyObject> get_False();

    ref<PyObject> add_PI();
    ref<PyObject> add_PO(PyObject* args, PyObject* kwds);
    ref<PyObject> add_Flop(PyObject* args, PyObject* kwds);
    ref<PyObject> add_Buf();

    void add_property(PyObject* o);
    void add_constraint(PyObject* o);
    void add_fair_property(PyObject* o);
    void add_fair_constraint(PyObject* o);

    ref<PyObject> n_PIs();
    ref<PyObject> n_POs();
    ref<PyObject> n_Flops();
    ref<PyObject> n_Bufs();
    ref<PyObject> n_Ands();

    ref<PyObject> n_properties();
    ref<PyObject> n_constraints();
    ref<PyObject> n_fair_properties();
    ref<PyObject> n_fair_constraints();

    ref<PyObject> get_PI(PyObject* o);
    ref<PyObject> get_PO(PyObject* o);
    ref<PyObject> get_Flop(PyObject* o);

    ref<PyObject> get_PIs();
    ref<PyObject> get_POs();
    ref<PyObject> get_Flops();
    ref<PyObject> get_Bufs();
    ref<PyObject> get_Ands();

    ref<PyObject> get_properties();
    ref<PyObject> get_constraints();
    ref<PyObject> get_fair_properties();
    ref<PyObject> get_fair_constraints();

    void clear_properties();
    void clear_constraints();
    void clear_fair_properties();
    void clear_fair_constraints();

    ref<PyObject> get_flop_init();
    ref<PyObject> get_names();

    ref<PyObject> copy();

    ref<PyObject> uporder(PyObject* args);

    void remove_unreach();
    void remove_bufs();

    void assure_pobs();

public:

    ZZ::Netlist N;

    ZZ::Vec<ZZ::Wire> _PIs;
    ZZ::Vec<ZZ::Wire> _POs;
    ZZ::Vec<ZZ::Wire> _Flops;

private:

    void ensure_netlist(ZZ::Wire w);

    void copy_props(ZZ::Netlist& M, const ZZ::WWMap& xlat);
    void copy_names(ZZ::Netlist& M, const ZZ::WWMap& xlat);
};

} // namesapce pyzz

#endif // #ifndef pyzz_netlist__H
