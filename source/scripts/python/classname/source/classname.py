#!/usr/bin/python3.5

class MyClass:
    "This is a class"
    a = 10;
    def func(self):
        print('Hello World');
        return 3;

def my_function(a: MyClass):
	return a.func();

def other_function(b: my_function):
    a = MyClass();
    return my_function(a);

def object_function(obj: object):
    return 3;
