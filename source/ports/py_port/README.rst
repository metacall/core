===============
M E T A C A L L
===============
A library for providing inter-language foreign function interface calls
-----------------------------------------------------------------------

Abstract
========

**METACALL** Python Port is the port of **METACALL** to Python
Programming Language. With **METACALL** Python Port you can
transparently execute code from Python to any programming language, for
example, calling JavaScript, NodeJS, Ruby or C# code from Python.

Install
========

Install MetaCall binaries first:

.. code:: console

   curl -sL https://raw.githubusercontent.com/metacall/install/master/install.sh | bash

Then install MetaCall Python package through MetaCall:

.. code:: console

   metacall pip3 install metacall

Example
========

``multiply.rb``

.. code:: ruby

    def multiply(left, right)
        return left * right
    end

``main.py``

.. code:: python

   from metacall import metacall_load_from_file, metacall

   metacall_load_from_file('rb', [ 'multiply.rb' ]);

   metacall('multiply', 3, 4); # 12

Running the example:

.. code:: console

   metacall main.py
