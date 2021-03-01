
import os
import ycm_core
 
flags=[
    '-Wall',
    '-Wextra',
    '-Werror',
    '-Wno-long-long',
    '-Wno-variadic-macros',
    '-fexceptions',
    '-DNDEBUG',
    '-std=c++11',
    '-x',
    'c++',
    #使用的库文件
    '-I',
    '/usr/include',
    '-isystem',
    '/usr/lib/gcc/x86_64-linux-gnu/9/include',
    '-isystem',
    '/usr/include/x86_64-linux-gnu',
    '-isystem'
    '/usr/include/c++/9',
    '-isystem',
    '/usr/include/c++/9/bits',
    #搜索当前目录下的文件
    '-isystem',
    '/home/fenghan/miniShell/' 
    ]
 
SOURCE_EXTENSIONS = [ '.cpp', '.cxx', '.cc', '.c', ]
 
def FlagsForFile( filename, **kwargs ):
  return {
    'flags': flags,
    'do_cache': True
}