# This test verifies that Python Loader loads local scripts before global ones
# test.py has the same file name (without extension) as the test module from Python stdlib
# so basically what happens is that Python is seeing an "import test", and loads first the
# stdlib module instead of the test.py, with the refactor done, the local file should be load first

def multiply_type(a: int, b: int) -> int:
	return a * b

print('Test: ' + str(multiply_type(33336666, 2)))
