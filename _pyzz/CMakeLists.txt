set(pyzz_sources
    pyzzwrapper.cc
    lit.cc
    netlist.cc
    solver.cc
    unroll.cc
    veciterator.cc
    wire.cc
    wmap.cc
    _pyzz.cpp)

python_add_module(_pyzz SHARED ${pyzz_sources})
target_compile_features(_pyzz PUBLIC cxx_lambdas)
target_link_libraries(_pyzz Bip-pic pywrapper)

python_add_module(_pyzz-static STATIC ${pyzz_sources})
target_compile_features(_pyzz-static PUBLIC cxx_lambdas)
target_link_libraries(_pyzz-static Bip pywrapper)
