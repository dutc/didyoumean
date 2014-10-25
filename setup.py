# didyoumean - "Did You Mean?" Functionality on AttributeError 

from setuptools import setup, find_packages, Extension
from os.path import join, dirname

didyoumean = Extension('didyoumean', 
                       sources=['src/didyoumean.c', 
                                'src/didyoumean-safe.c'],
                       include_dirs=['src'], 
                       depends=['src/didyoumean-safe.h'])

setup(
    name='dutc-didyoumean',
    version='0.1.2',
    description='"Did You Mean?" on AttributeError',
    long_description=''.join(open(join(dirname(__file__),'README.md'))),
    author='James Powell',
    author_email='james@dontusethiscode.com',
    url='https://github.com/dutc/didyoumean',
    packages=find_packages(exclude=['*demos*']),
    ext_modules=[didyoumean],
)
