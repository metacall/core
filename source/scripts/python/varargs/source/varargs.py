def test_args(*args):
    assert len(args)==3
    assert args[0]== 1

    assert args[1]==2
    assert args[2]==3

    return sum(args)

def test_kwargs(**kwargs):
    assert len(kwargs)==2
    assert "x" in kwargs
    assert "y" in kwargs
    assert kwargs["x"]==10
    assert kwargs["y"]==20
    return sum(kwargs.values())

def test_varargs(*args, **kwargs):
    assert len(args)==2
    assert args[0]==100
    assert args[1]==200

    assert len(kwargs)==2
    assert "a" in kwargs
    assert "b" in kwargs
    assert kwargs["a"]== 1
    assert kwargs["b"]==2
    return sum(args)+ sum(kwargs.values)

def test_mixed_args(a, b, *args):
    assert a == 1
    assert b == 2
    assert len(args) == 2
    assert args[0] == 3
    assert args[1] == 4
    return a + b + sum(args)

def test_mixed_kwargs(a, b, **kwargs):
    assert a == 1
    assert b == 2
    assert len(kwargs) == 2
    assert "x" in kwargs
    assert "y" in kwargs
    assert kwargs["x"] == 10
    assert kwargs["y"] == 20
    return a + b + sum(kwargs.values())

def test_mixed(a, b, *args, **kwargs):
    assert a==10
    assert b==20

    assert len(args)==2
    assert len(kwargs)==2
    assert args[0]==30
    assert args[1]==40

    assert "key" in kwargs 
    assert kwargs["key"]==50
    return a+b+sum(args)+sum(kwargs.values())

    
