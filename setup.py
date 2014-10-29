# maybeyoumeant - "Did You Mean?" Functionality on AttributeError

from setuptools import setup, find_packages, Extension
from os.path import join, dirname

maybeyoumeant = Extension('maybeyoumeant',
                       sources=['src/maybeyoumeant.c',
                                'src/hook.c',
                                'src/levenshtein.c',
                                'src/safe_PyObject_Dir.c'],
                       include_dirs=['src'],
                       depends=['src/safe_PyObject_Dir.h'])

setup(
    name='maybeyoumeant',
    version='0.1.6',
    description='"Meant You Meant" on AttributeError',
    long_description=''.join(open(join(dirname(__file__),'README.md'))),
    author='James Powell',
    author_email='james@dontusethiscode.com',
    url='https://github.com/dutc/maybeyoumeant',
    packages=find_packages(exclude=['*demos*']),
    ext_modules=[maybeyoumeant],
)
