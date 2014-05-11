#!/usr/bin/env python

import os
import sys
import shutil

arg = None

if len(sys.argv) > 1:
  arg = sys.argv[1]

if arg == 'clean':
  os.system('../../BUILD/zb ,realclean')
  os.system('python setup.py develop --uninstall')
  shutil.rmtree('build', ignore_errors=True)
  shutil.rmtree('dist', ignore_errors=True)
  shutil.rmtree('bdist', ignore_errors=True)
  sys.exit(0)

elif arg=='quick':
  os.putenv('PYZZ_ZB_CONFIG', 'quick_i')
  zb_opt = 'qi'

elif arg=='debug':
  os.putenv('PYZZ_ZB_CONFIG', 'debug_i')
  zb_opt = 'di'

elif arg=='profile':
  os.putenv('PYZZ_ZB_CONFIG', 'profile_i')
  zb_opt = 'pi'

else:
  os.putenv('PYZZ_ZB_CONFIG', 'release_i')
  zb_opt = 'ri'

rc = os.system('../../BUILD/zb %s'%zb_opt)

if rc != 0:
  sys.exit(rc)

shutil.rmtree('build', ignore_errors=True)

if sys.platform.startswith('darwin'):
  develop_args = ''
else:
  develop_args = '--user'

rc = os.system('python setup.py develop %s'%develop_args)

sys.exit(rc)
