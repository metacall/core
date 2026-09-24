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
def return_small_number():
    return 42
def return_negative_int():
    return -1
def return_large_number():
    return 9223372036854775807  # i64::MAX