from imported_fns import imported_fn

def hello_scala_from_python(a, b):
    return a + b

def fail():
    raise Exception('Failed!')

def apply_fn_to_str(fn):
    return fn('hellooo')

def apply_fn_to_one(fn):
    return fn(1)

def get_function_test(a):
    return a
