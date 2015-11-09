#include <Python.h>

namespace pyzz
{
void zz_init();
}

int main(int argc, char *argv[])
{
    PyImport_AppendInittab("_pyzz", pyzz::zz_init);
    return Py_Main(argc,argv);
}
