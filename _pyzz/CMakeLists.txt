python_add_module(
    _pyzz
    SHARED
        pyzzwrapper.cc
        lit.cc
        netlist.cc
        solver.cc
        unroll.cc
        veciterator.cc
        wire.cc
        wmap.cc
        _pyzz.cpp
    )

target_compile_features(_pyzz PUBLIC cxx_lambdas)
target_link_libraries(_pyzz Bip-pic pywrapper)
