### Disclaimer

dutc = Don't Use This Code

# Did You mean?

This module implements "Did You Mean?" functionality on AttributeError.

(It's not what it does but how it does it!)

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

Importing and enabling `didyoumean`:

```python
>>> import didyoumean
>>> foo.baz
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: 

Maybe you meant: .bar

>>>
```

### How Do I Turn It Off?

You can't.

### Is It Portable?

Probably not.

### Does this approach work on ...?

It works on Linux. It works on x86_64. 

It's unfortunate that it works anywhere.

### Should I Use This Code?

Definitely not.

### License

License (GPLv3)

Copyright (C) 2013  James Powell <james@dontusethiscode.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
