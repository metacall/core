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