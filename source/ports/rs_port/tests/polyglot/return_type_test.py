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

def test_pointer(pointer):
    return pointer

def test_buffer(buffer):
    return buffer