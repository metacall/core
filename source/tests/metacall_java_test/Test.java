public class Test {
  public int INT_NS = 123433;
  public char CHAR_NS = 'Z';
  public String STRING_NS = "NS string";

  public static boolean BOOL_TEST = true;
  public static char CHAR_TEST = 'A';
  public static short SHORT_TEST = 100;
  public static int INT_TEST = 30;
  public static long LONG_TEST = 10000009;
  public static float FLOAT_TEST = (float) 10.1233;
  public static double DOUBLE_TEST = 10.21324434;
  public static String STRING_TEST = "Hello";

  public Test(int val, int s) {
    System.err.println("Test const Called with " + val + ", " + s);
    INT_TEST = val;
  }

  public Test(boolean b, char c, short sh, int i, long l, float f, double d, String s) {
    System.err.println("Full Test constructor Called " + s);
    BOOL_TEST = b;
    CHAR_TEST = c;
    SHORT_TEST = sh;
    INT_TEST = i;
    LONG_TEST = l;
    FLOAT_TEST = f;
    DOUBLE_TEST = d;
    STRING_TEST = s;
  }

  public static void testFunctNew() {
    System.out.println("hello Mr bean");
  }

  public static int testFunct(String s) {
    System.out.println("hello " + s);
    return 0;
  }

  public static void main(String[] args) {
    System.out.println("Hello World");
  }
}
