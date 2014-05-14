#ifndef pyzz_unroll__H
#define pyzz_unroll__H

#include "pyzzwrapper.h"

namespace ZZ
{

struct GLitSeen :
    IntSeen<GLit, MkIndex_GLit<false>>
{
    GLitSeen() :
        IntSeen<GLit, MkIndex_GLit<false>>()
    {
    }
};

} // namespace ZZ

namespace pyzz
{

class Netlist;

class Unroll :
    public type_base_with_new<Unroll>
{
public:

    Unroll(borrowed_ref<Netlist> N, bool init);
    ~Unroll();

    static void initialize(PyObject* module);
    static void construct(Unroll* p, PyObject* args, PyObject*);

    Py_ssize_t mp_length();
    ref<PyObject> mp_subscript(PyObject* o);
    int sq_contains(PyObject* pkey);

private:

    bool _init;

    ref<PyObject> _N;
    ZZ::NetlistRef N;

    ref<PyObject> _F;
    ZZ::NetlistRef F;

    ZZ::Vec<ZZ::GLitSeen> _visited;
    ZZ::Vec<ZZ::WWMap> _maps;

    struct stack_elem
    {
        stack_elem(ZZ::GLit lit_, uint k_) :
            lit(lit_),
            k(k_)
        {
        }

        stack_elem mark() const
        {
            return stack_elem(~lit, k);
        }

        stack_elem unmark() const
        {
            return stack_elem(+lit, k);
        }

        bool is_marked() const
        {
            return sign(lit);
        }

        ZZ::GLit lit;
        uint k;
    };

    ZZ::Vec<stack_elem> _dfs_stack;

    void ensure_frame(uint k);

    bool is_visited(stack_elem e);
    bool visit(stack_elem e);

    void push_children(stack_elem e);

    ZZ::Wire unroll(ZZ::GLit w, uint k);
    void unroll_wire(stack_elem e);
};

} // namesapce pyzz

#endif // #ifndef pyzz_unroll__H
