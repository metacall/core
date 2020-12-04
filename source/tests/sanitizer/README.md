# Sanitizer Supressions

The `lsan.supp` must not have comments nor empty lines in between in order to fully support gcc and clang. Here is the detailed suppression list per runtime:

```
# Python
leak:_PyObject_Malloc
leak:_PyObject_Realloc
leak:PyThread_allocate_lock
leak:libpython*

# NodeJS
# Suppress small (intentional) leaks in glibc
leak:libc.so

# Ruby
leak:libruby*
```
