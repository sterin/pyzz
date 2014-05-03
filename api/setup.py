import os
import platform

from distutils.core import setup
from distutils.extension import Extension
from distutils.command.build_ext import build_ext

config = 'release_i'

class build_ext_subclass( build_ext ):
    def build_extensions(self):
        args = ['../../BUILD/zz_gdep_link'] + self.compiler.compiler_cxx[1:]
        self.compiler.compiler_cxx = args
        build_ext.build_extensions(self)

libraries = [
    "ZZ_pyzz.api", 
    "ZZ_Bip", 
    "ZZ_CnfMap", 
    "ZZ_AbcInterface", 
    "ZZ_Bip.Common", 
    "ZZ_MetaSat", 
    "SiertSat", 
    "ZZ_MetaSat.AbcSat", 
    "ZZ_MetaSat.MiniSat2", 
    "ZZ_BFunc", 
    "ZZ_Npn4", 
    "ZZ_MiniSat", 
    "ZZ_Abc", 
    "ZZ_Netlist", 
    "ZZ_Generics", 
    "ZZ_CmdLine", 
    "ZZ_Prelude",
    ]

extra_link_args = ['-Wl,-whole-archive'] + [ '-l%s'%l for l in libraries ] + ['-Wl,-no-whole-archive']

if platform.system()=='Linux':
    extra_link_args.append( '-fPIC' )
    extra_link_args.append( '-lrt' )

with os.popen("uname -mrs", "r") as p:
    machine = p.readline().rstrip().replace(' ', '-')

library_dirs = [ "../../lib/%s/%s"%(machine, config) ]

ext = Extension(
    '_pyzz',
    ['_pyzz.cpp'],
    library_dirs=library_dirs,
    extra_link_args=extra_link_args
    )

setup(
    name='pyzz',
    version='1.0',
    ext_modules=[ext],
    py_modules=['pyzz'],
    cmdclass={'build_ext':build_ext_subclass}
    )
