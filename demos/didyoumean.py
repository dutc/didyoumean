#!/usr/bin/env python

class Foo(object):
    def bar(self):
       pass

if __name__ == '__main__':
    foo = Foo()

    print foo.bar

    try:
        foo.baz
    except Exception as e:
        print e

    import didyoumean

    print foo.bar

    try:
        foo.baz
    except Exception as e:
        print e
