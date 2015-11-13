set(pyzz_sources
    pyzzwrapper.cpp
    lit.cpp
    netlist.cpp
    solver.cpp
    unroll.cpp
    veciterator.cpp
    wire.cpp
    wmap.cpp
    _pyzz.cpp)

pyabc_python_add_module(_pyzz SHARED ${pyzz_sources})
target_compile_features(_pyzz PUBLIC cxx_lambdas)
target_link_libraries(_pyzz PUBLIC Bip-pic pywrapper)

pyabc_python_add_module(_pyzz-static STATIC ${pyzz_sources})
target_compile_features(_pyzz-static PUBLIC cxx_lambdas)
target_link_libraries(_pyzz-static PUBLIC Bip pywrapper)

add_executable(pyzz.exe main.cpp)
target_link_libraries(pyzz.exe PRIVATE _pyzz-static)
