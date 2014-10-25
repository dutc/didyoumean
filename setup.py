# didyoumean - "Did You Mean?" Functionality on AttributeError 

from setuptools import setup, find_packages, Extension
from os.path import join, dirname

didyoumean = Extension('didyoumean', 
                       include_dirs=['include'], 
                       sources=['src/didyoumean.c', 
                                'src/didyoumean-safe.c'])

if __name__ == '__main__':
    setup(
        name='dutc-didyoumean',
        version='0.1.0',
        description='"Did You Mean?" on AttributeError',
        long_description=''.join(open(join(dirname(__file__),'README.md'))),
        author='James Powell',
        author_email='james@dontusethiscode.com',
        url='https://github.com/dutc/didyoumean',
        packages=find_packages(exclude=['*demos*']),
        ext_modules=[didyoumean],
    )

