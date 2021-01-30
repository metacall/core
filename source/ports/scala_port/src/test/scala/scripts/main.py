from imported_fns import imported_fn


def hello_scala_from_python(a, b):
    return a + b


def fail():
    raise Exception('Failed!')


def apply_fn_to_str(fn):
    return fn('hellooo')


def apply_fn_to_one(fn):
    return fn(1)


def apply_fn_to_two(fn):
    return fn(1, 3)


def get_function_test(a):
    return a


def sumList(list: list):
    return sum(list)


def big_fn(i: int, s: str, f: float):
    return i + len(s) + f
