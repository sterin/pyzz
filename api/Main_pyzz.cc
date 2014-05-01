#include "pyzzwrapper.h"

int
main(int argc, char *argv[])
{
    ZZ_Init

    py::initialize interpreter(argv[0]);

    pyzz::init();

    return Py_Main(argc, argv);
}
