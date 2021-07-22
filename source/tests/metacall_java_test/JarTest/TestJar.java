package src.JarTest;

public class TestJar {
  public static String jarTestString = "This is a static Jar String";

  public static void main(String[] args) {
    TesterJar2 t = new TesterJar2();
    t.testCall();
    System.out.println("Hello World");
  }
}