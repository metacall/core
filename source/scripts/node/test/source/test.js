class Fibonacci {
  fib_impl(n) {
    if (n <= 2) {
      return 1;
    } else {
      return fib_impl(n - 2) + fib_impl(n - 1);
    }
  }

  static fib(n) {
    console.log("Fibbonaci of " + n + " = " + fib_impl(n));
  }
}

class Test {

  constructor (num, str, cls, cls_args) {
    this.int = 231;
    this.b =  100;
    this.hello_str = "hello!";

    this.arr = [ 'K', 'G', 50, 100, "Hello", "world"];
    this.num = num;
    this.str = str;
    this.cls = Fibonacci;

    // TODO: Add test for this
    this.obj = new cls(...cls_args);
    // END-TODO
    
    console.log("NodeJs Loader: Test constructor Called!");
  }

  newFibonacci() {
    return new Fibonacci;
  }

  return_class() {
    return Fibonacci;
  }

  return_new_obj() {
    return this.obj.return_bye('LBryan');
  }

  print() {
    console.log(this.str);
  }

  // Static method with no parameter and void return type
  static hello(s) {
    console.log("hello, world! running on " + s);
    return 10;
  }

  static sumArray(i) {
    sum = 0;

    for (let c of i)
      sum += c;

    return sum;
  }

}

module.exports = Test