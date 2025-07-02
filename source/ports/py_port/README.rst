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
=======

Install MetaCall binaries first:

.. code:: console

   curl -sL https://raw.githubusercontent.com/metacall/install/master/install.sh | bash

Then install MetaCall Python package through MetaCall:

.. code:: console

   pip3 install metacall

Example
=======

Calling Ruby from Python
------------------------

``multiply.rb``

.. code:: ruby

    def multiply(left, right)
        return left * right
    end

``main.py``

.. code:: python

   from metacall import metacall_load_from_file, metacall

   metacall_load_from_file('rb', [ 'multiply.rb' ])

   metacall('multiply', 3, 4); # 12

Running the example:

.. code:: console

   python3 main.py

Using pointers (calling to a C library)
---------------------------------------

For a simple case, let's imagine that we have a simple C function that
has an 'in' parameter and we want to pass a pointer to a long, from
Python side, and then store some value there for reading it later on.
Let's assume we have a ``loadtest.h`` and ``libloadtest.so`` and a C
function from this library could be this one:

.. code:: c

   void modify_int_ptr(long *l)
   {
       *l = 111;
   }

Now if we want to call it from Python side, we should do the following:

.. code:: py

   from metacall import metacall_load_from_package, metacall, metacall_value_reference, metacall_value_dereference

   # Load the library (we can configure the search paths for the .so and .lib with metacall_execution_path)
   # metacall_execution_path('c', '/usr/local/include')
   # metacall_execution_path('c', '/usr/local/lib')
   metacall_load_from_package('c', 'loadtest')

   # Create value pointer (int *)
   int_val = 324444
   int_val_ref = metacall_value_reference(int_val)

   # Pass the pointer to the function
   metacall('modify_int_ptr', int_val_ref)

   # Get the value from pointer
   int_val_deref = metacall_value_dereference(int_val_ref)
   print(int_val_deref, '==', 111)

For a more complex case, where we have an in/out parameter, for example
an opaque struct that we want to alloc from C side. First of all, with
the following header ``loadtest.h``:

.. code:: c

   #ifndef LIB_LOAD_TEST_H
   #define LIB_LOAD_TEST_H 1

   #if defined(WIN32) || defined(_WIN32)
       #define EXPORT __declspec(dllexport)
   #else
       #define EXPORT __attribute__((visibility("default")))
   #endif

   #ifdef __cplusplus
   extern "C" {
   #endif

   #include <cstdint>

   typedef struct
   {
       uint32_t i;
       double d;
   } pair;

   typedef struct
   {
       uint32_t size;
       pair *pairs;
   } pair_list;

   EXPORT int pair_list_init(pair_list **t);

   EXPORT double pair_list_value(pair_list *t, uint32_t id);

   EXPORT void pair_list_destroy(pair_list *t);

   #ifdef __cplusplus
   }
   #endif

   #endif /* LIB_LOAD_TEST_H */

With the following implementation ``loadtest.cpp``:

.. code:: c

   #include "loadtest.h"

   int pair_list_init(pair_list **t)
   {
       static const uint32_t size = 3;

       *t = new pair_list();

       (*t)->size = size;
       (*t)->pairs = new pair[(*t)->size];

       for (uint32_t i = 0; i < size; ++i)
       {
           (*t)->pairs[i].i = i;
           (*t)->pairs[i].d = (double)(((double)i) * 1.0);
       }

       return 0;
   }

   double pair_list_value(pair_list *t, uint32_t id)
   {
       return t->pairs[id].d;
   }

   void pair_list_destroy(pair_list *t)
   {
       delete[] t->pairs;
       delete t;
   }

In this case the structs are not opaque, but they can be opaque and it
will work in the same way. Now, we can call those functions in the
following manner:

.. code:: py

   from metacall import metacall_load_from_package, metacall, metacall_value_create_ptr, metacall_value_reference, metacall_value_dereference

   metacall_load_from_package('c', 'loadtest')

   # Create a pointer to void* set to NULL
   list_pair = metacall_value_create_ptr(None)

   # Create a reference to it (void**)
   list_pair_ref = metacall_value_reference(list_pair)

   # Call the function
   result = metacall('pair_list_init', list_pair_ref)

   # Get the result updated (struct allocated)
   list_pair = metacall_value_dereference(list_pair_ref)

   # Pass it to a function
   result = metacall('pair_list_value', list_pair, 2)

   # Destroy it
   metacall('pair_list_destroy', list_pair)

   # Here result will be 2.0 because is the third element in the array of pairs inside the struct
   print(result, '==', 2.0)
