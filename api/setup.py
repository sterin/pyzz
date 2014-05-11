import os
import platform

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

def pyzz_lib():

    def parts(path):

        while True:

            if os.path.exists(os.path.join(path, '.zb_root')):
                break

            path, tail = os.path.split(path)

            if not tail:
                raise RuntimeError("could not find .zb_root")

            yield tail

    return "ZZ_" + ".".join(reversed(list(parts(os.getcwd()))))

libraries = [
    pyzz_lib(),
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

class build_ext_subclass( build_ext ):

    def build_extensions(self):

        if 'PYZZ_ZB_CONFIG' in os.environ:
            config = os.environ['PYZZ_ZB_CONFIG']

        elif self.debug:
            config = 'quick_i'
            
        else:
            config = 'release_i'

        library_dir = "../../lib/%s/%s"%(machine, config)

        for ext in self.extensions:
            ext.library_dirs.append(library_dir)

        args = ['../../BUILD/zz_gdep_link'] + self.compiler.compiler_cxx[1:]
        self.compiler.compiler_cxx = args

        return build_ext.build_extensions(self)

ext = Extension(
    'pyzz._pyzz',
    ['_pyzz.cpp'],
    extra_link_args=extra_link_args
    )

setup(
    name='pyzz',
    version='1.0',
    ext_modules=[ext],
    packages=['pyzz'],
    cmdclass={'build_ext':build_ext_subclass}
    )
