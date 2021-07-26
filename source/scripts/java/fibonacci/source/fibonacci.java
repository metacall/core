public class Fibonacci {
	public static int fib_impl(int n) {
		if (n <= 2) {
			return 1;
		} else {
			return fib_impl(n - 2) + fib_impl(n - 1);
		}
	}

	public static void fib(int n) {
		System.out.println("Fibbonaci of " + n + " = " + fib_impl(n));
	}
}
