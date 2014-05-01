#include <cstdio>

namespace ZZ
{
    void zzInitialize(bool finalize=false);
}

namespace pyzz
{
    void init();
}

extern "C"
void init_pyzz()
{
    ZZ::zzInitialize();
    pyzz::init();
}
