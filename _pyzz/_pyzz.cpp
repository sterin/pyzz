#include "pywrapper.h"

namespace pyzz
{
    void zz_init();
}

PyMODINIT_FUNC 
init_pyzz()
{
    pyzz::zz_init();
}
