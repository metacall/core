#!/usr/bin/env python3

class MyClass():
    # String __doc__ inside MyClass.__dict__
    "This is a class"
    a = 10
    b = 20
    helloString = "Hello World"

    def __init__(self, param1=11, param2=21):
        self.a = param1
        self.b = param2

    def hello(self):
        print(self.helloString)
        return 0
    def return_bye(self, name):
        return "bye " + name
    @staticmethod
    def static(return_val):
        return return_val

"""
>>> return_function_bound_method_param(MyClass())
<bound method MyClass.hello of <__main__.MyClass object at 0x7f18cdb55e80>>
>>> return_function_bound_method_param(MyClass)
<function MyClass.hello at 0x7f18cdb81700>
"""


def return_object_function():
    # Returns an instance
    return MyClass()


def return_class_function():
    # type(MyClass) == type
    # Returns class definition
    return MyClass


def return_itself(a: MyClass):
    # Try passing MyClass(), MyClass and MyClass.a or .hello
    return a


def return_bound_method_param(a):
    return a.hello


def return_object_bound_method_call():
    a = MyClass()
    return return_bound_method_param(a)


def function_returns_object_new_local_variable():
    x = MyClass()
    x.b = "Overriding class definition *b*"
    x.nonexistent = "this was not defined in class"
    x.return_bye = x.hello
    return x.return_bye


def return_object_bound_method_new_object():
    x = MyClass()
    return x.hello
