public class Test {
  // Non static data types
  public int INT_NS = 231;
  public char CHAR_NS = 'Z';
  public String STRING_NS = "NS string";

  // Non static Arrays
  public char[] CHAR_TEST_Arr_NS = new char[] { 'K', 'G' };
  public int[] INT_TEST_Arr_NS = new int[] { 50, 100 };
  public String[] STRING_TEST_Arr_NS = new String[] { "Hello", "world" };

  // Static data types
  public static boolean BOOL_TEST = true;
  public static char CHAR_TEST = 'A';
  public static short SHORT_TEST = 100;
  public static int INT_TEST = 30;
  public static long LONG_TEST = 10000009;
  public static float FLOAT_TEST = (float) 10.1233;
  public static double DOUBLE_TEST = 10.21324434;
  public static String STRING_TEST = "Hello";
  public static Class<?> CLASS_TEST = Test.class;

  // TODO: Add test for this
  public static Object OBJECT_TEST = new Object();
  // END-TODO

  // Static array data types
  public static boolean[] BOOL_TEST_Arr = new boolean[] { true, false };
  public static char[] CHAR_TEST_Arr = new char[] { 'K', 'G' };
  public static short[] SHORT_TEST_Arr = new short[] { 12, 13 };
  public static int[] INT_TEST_Arr = new int[] { 30, 12 };
  public static long[] LONG_TEST_Arr = new long[] { 8765434, 12345678 };
  public static float[] FLOAT_TEST_Arr = new float[] { 1, 2 };
  public static double[] DOUBLE_TEST_Arr = new double[] { 1.32, 2.12 };
  public static String[] STRING_TEST_Arr = new String[] { "Hello", "world" };
  public static Class<?>[] CLASS_TEST_Arr = new Class<?>[] { Test.class };

  // TODO: Add test for this
  public static String[][] STRING_ARRAY_ARRAY_TEST = new String[][]{new String[]{"String 1"}, new String[]{"String 2"}, new String[]{"String 3"}};
  // END-TODO

  public Test(int val, int s) {
    System.err.println("Test const Called with " + val + ", " + s);
    INT_TEST = val;
  }

  public Test(String[] strArr) {
    System.out.print("Array Constructor = ");

    for (String s : strArr)
      System.out.print(s + " ");

    System.out.println();
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

  public int TestNonStaticInt(int a) {
    System.out.println("NS int " + a);
    return a * 10;
  }

  public static void testFunctNew() { // static method with no parameter and void return type
    System.out.println("hello Mr bean");
  }

  public static int testFunct(String s, int a) { // static method with multiple parameters
    System.out.println("hello " + s + " " + a);
    return 10;
  }

  public static int testIntArrayAdd(int[] i) { // static method with array parameter and primitive return type
    int sum = 0;
    for (int c : i)
      sum += c;
    return sum;
  }

  public static String[] testStringArrayFunction(String[] s) {// static method with ARRAY parameter and ARRAY return
    System.out.println("String function ");
    for (String st : s) {
      System.out.println(st);
    }

    return new String[] { "Test", "worked" };
  }

  // TODO: Add tests for this
  public static String[] testFunctNew2() {
    return new String[]{ "hello", "Mr", "bean" };
  }

  public static String[][] testFunctNew3() {
    return new String[][]{ new String[]{ "hello" },  new String[]{ "Mr" },  new String[]{ "bean" } };
  }

  public static void yeet(String[] asd) {
    System.out.println("yeet");
  }
  // END-TODO

  public static void main(String[] args) {
    System.out.println("Hello World");
  }
}
