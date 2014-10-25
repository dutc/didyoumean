#### Disclaimer

dutc = Don't Use This Code (!!)

# Did You mean?

This module implements "Did You Mean?" functionality on AttributeError.

*It's not what it does but how it does it!*

```python
>>> class Foo(object):
...  def bar(self): pass
... 
>>> foo = Foo()
```

Without `didyoumean`:

```python
>>> foo.baz
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: 'Foo' object has no attribute 'baz'
```

```python
>>> getattr(foo, 'baz')
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: 'Foo' object has no attribute 'baz'
```

After importing `didyoumean`:

```python
>>> import didyoumean
>>> foo.baz
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: 'Foo' object has no attribute 'baz'

Maybe you meant: .bar
```

```python
>>> import didyoumean
>>> getattr(foo, 'baz')
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: 'Foo' object has no attribute 'baz'

Maybe you meant: .bar
```

It works on old-style classes, new-style classes, type objects, builtins, everything.

```python
>>> # old-style class
... 
>>> class Foo:
...   def bar(self): pass
... 
>>> Foo.baz
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: class Foo has no attribute 'baz'

Maybe you meant: .bar
```

```python
>>> # new-style class
... 
>>> class Foo(object):
...   def quux(self): pass
... 
>>> foo = Foo()
>>> foo.baz
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: 'Foo' object has no attribute 'baz'

Maybe you meant: .quux
```

```python
>>> # types
... 
>>> class Bar(Foo):
...   def xyzzy(self): pass
... 
>>> Bar.qux
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: type object 'Bar' has no attribute 'qux'

Maybe you meant: .quux

>>> Bar.xyzz
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: type object 'Bar' has no attribute 'xyzz'

Maybe you meant: .xyzzy
```

```python
>>> # builtins & everything else?
... 
>>> object._new__
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: type object 'object' has no attribute '_new__'

Maybe you meant: .__new__

>>> object()._new__
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: 'object' object has no attribute '_new__'

Maybe you meant: .__new__
```

```python
>>> # works with getattr
... 
>>> getattr(object, '_new__')
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: type object 'object' has no attribute '_new__'

Maybe you meant: .__new__
```

```python
>>> # works with getattr
... #   ... even if you don't want it to!
... 
>>> g = getattr
>>> import didyoumean
>>> getattr(object, '_new__')
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: type object 'object' has no attribute '_new__'

Maybe you meant: .__new__

>>> g(object, '_new__')
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: type object 'object' has no attribute '_new__'

Maybe you meant: .__new__
```

#### How do I install it?

```shell
$ pip install dutc-didyoumean
```

#### How do I turn it off?

Restart your interpreter.

#### How does it work?

Well, see, that's the fun of it.

#### Is this approach ‘portable’?

Probably not.

#### Does this approach work on ...?

It works on Python 2. It works on Linux. It works on x86_64. It's unfortunate that it works anywhere.

#### Should I use this code?

Definitely not.

### License (GPLv3)

Copyright © 2014 James Powell <james@dontusethiscode.com>

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <http://www.gnu.org/licenses/>.
