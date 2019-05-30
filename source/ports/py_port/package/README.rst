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
example, calling JavaScript (NodeJS) code from Python.

``sum.js``

.. code:: javascript

   module.exports = function sum(a, b) {
       return a + b;
   };

``main.py``

.. code:: python

   from metacall import metacall_load_from_file, metacall

   metacall_load_from_file('node', [ 'sum.js' ]);

   metacall('sum', 3, 4); // 7
