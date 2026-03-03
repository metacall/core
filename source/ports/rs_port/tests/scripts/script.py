class TestClass:
    hi = "there!"
    hi2 = "x"

    @staticmethod
    def hi_function():
        return "there!";
    def get_hi(self):
        return self.hi;

def test_class():
    return TestClass
def test_object():
    return TestClass()
def return_the_argument_py(argument):
    return argument
def return_negative_int_py():
    return -1
def return_large_number():
    return 5000000000  # bigger than u32::MAX