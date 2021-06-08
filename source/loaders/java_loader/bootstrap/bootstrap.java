import java.io.File;
import javax.tools.*;
import java.util.*;

import java.lang.reflect.Method;
import java.lang.Object;

import java.nio.file.Path;
import java.nio.file.Paths;

class Handle {
  HashMap<String, Class<?>> handleMap = new HashMap<String, Class<?>>();

  public void addClass(String name, Class<?> classObj) {
    handleMap.put(name, classObj);
  }

  public Class<?> getClassFromName(String name) {
    return handleMap.get(name);
  }

  public String[] getStringArray() {
    String[] strArr = new String[handleMap.size()];
    int i = 0;
    for (Map.Entry<String, Class<?>> handle : handleMap.entrySet()) {
      strArr[i++] = handle.getKey();
    }

    return strArr;
  }
}

public class bootstrap {
  private static Handle handleArray = new Handle(); // Global Handle Class to store classes and names

  public static void callFunction(String classname, String functionName) {
    Class<?> c = handleArray.getClassFromName(classname);

    try {
      Method m = c.getDeclaredMethod(functionName, new Class[] { String[].class });
      m.invoke(null, new Object[] { null });

    } catch (Exception e) {
      System.err.println("CallFunction" + e);
    }
  }

  public static String[] loadFromFile(String[] paths) {
    // load all scripts and store them into a Handle class, then return it
    for (int i = 0; i < paths.length; i++) {
      System.out.println("Path provided " + paths[i]);

      try {
        JavaCompiler compiler = ToolProvider.getSystemJavaCompiler();
        DiagnosticCollector<JavaFileObject> ds = new DiagnosticCollector<>();
        StandardJavaFileManager mgr = compiler.getStandardFileManager(ds, null, null);
        Iterable<String> classOutputPath = Arrays
            .asList(new String[] { "-d", "/home/ketangupta34/Desktop/core/source/scripts/java/fibonacci/source" });

        File file1 = new File(paths[i]);
        Iterable<? extends JavaFileObject> sources = mgr.getJavaFileObjectsFromFiles(Arrays.asList(file1));
        JavaCompiler.CompilationTask task = compiler.getTask(null, mgr, ds, classOutputPath, null, sources);
        Boolean call = task.call(); // main method to compile the file into class

        if (call) {
          System.out.println("Compilation Successful");
          Path path = Paths.get(file1.getCanonicalPath());
          String classname = path.getFileName().toString().split(".java")[0];

          handleArray.addClass(classname, Class.forName(classname));
        } else {
          System.out.println("Compilation Failed");
        }

        for (Diagnostic<? extends JavaFileObject> d : ds.getDiagnostics()) { // diagnostic error printing
          System.out.format("DIAGNOSTIC Line: %d, %s in %s", d.getLineNumber(), d.getMessage(null),
              d.getSource().getName());
        }

        mgr.close();
        System.out.print("\n");

      } catch (Exception e) {
        System.err.println("Load Function" + e);
      }
    }

    return handleArray.getStringArray();
  }

  public static void DiscoverData(String classname) {
    // for each loaded .java file in the Handle list, get the DiscoverData, which is
    // another class with the list of classes and methods etc
    Class<?> hClass = handleArray.getClassFromName(classname);

    System.out.println("ClassName: " + hClass.getName());

    Method[] methods = hClass.getDeclaredMethods();

    for (Method method : methods) {
      System.out.println("Name of the method: " + method.getName());

      Class<?>[] parameters = method.getParameterTypes();
      if (parameters.length == 0)
        System.out.println("\tparameter: none");
      for (Class<?> parameter : parameters) {
        System.out.println("\tparameter: " + parameter.getSimpleName());
      }
      System.out.println("\tReturn Type: " + method.getReturnType() + "\n");

    }
  }
}