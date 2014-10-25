# didyoumean - "Did You Mean?" Functionality on AttributeError 

from setuptools import setup, find_packages, Extension

dutc_didyoumean = Extension('dutc_didyoumean', 
                        sources=['src/didyoumean.c', 
                                 'src/didyoumean-safe.c'])

if __name__ == '__main__':
    setup(
        name='dutc_didyoumean',
        version='0.1.0',
        description='"Did You Mean?" on AttributeError',
        long_description=''.join(open('./README.md')),
        author='James Powell',
        author_email='james@dontusethiscode.com',
        url='https://github.com/dutc/didyoumean',
        packages=find_packages(exclude=['*demos*']),
        ext_modules=[dutc_didyoumean],
    )

