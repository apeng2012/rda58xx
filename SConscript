# RT-Thread building script for bridge

from building import *

cwd     = GetCurrentDir()
src     = Glob('*.c') + Glob('*.cpp')
path    = [cwd]

group = DefineGroup('rda58xx', src, depend = ['PKG_USING_RDA58XX'], CPPPATH = path)

Return('group')
