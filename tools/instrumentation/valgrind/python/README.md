# Python + Valgrind

Builds Python 3.13 from Debian source with Valgrind support and runs the MetaCall Python port test suite under Valgrind.

## Build and run

```bash
# From the repo root
docker build -t metacall-python-valgrind -f Dockerfile .
docker run --rm metacall-python-valgrind
```

## With aggressive ruby suppressions

```bash
docker run --rm metacall-python-valgrind valgrind --leak-check=full --num-callers=30 \
	--suppressions=/app/memcheck/valgrind-ruby.supp \
	--suppressions=python-valgrind.supp \
	python3.13 test.py
```

## Python build flags

- `--with-pydebug` -- enables debug hooks
- `--without-pymalloc` -- disables Python small object allocator
- `--with-valgrind` -- cleaner memory tracking
- `Py_USING_MEMORY_DEBUGGER` uncommented in `Objects/obmalloc.c`
