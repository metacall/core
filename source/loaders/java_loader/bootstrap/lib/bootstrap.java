import java.io.File;

import javax.tools.*;
import java.util.*;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.net.URL;
import java.net.URLClassLoader;
import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

import java.nio.file.Path;
import java.nio.file.Paths;

public class bootstrap {
  private static Set<String> executionPath = new HashSet<String>();

  public static Class<?> FindClass(String name) {
    System.out.println("Finding Class " + name);

    try {
      URL[] urlArr = new URL[executionPath.size()];
      int i = 0;
      for (String x : executionPath)
        urlArr[i++] = new File(x).toURI().toURL();

      Class<?> cls = Class.forName(name, true, new URLClassLoader(urlArr));
      return cls;

    } catch (Exception e) {
      System.out.println("Find Class Error" + e);
    }
    return null;
  }

  public static int java_bootstrap_execution_path(String path) {
    System.out.println("bootstraping Execution path = " + path);
    executionPath.add(path);

    try {
      URL execPathFile = new File(path).toURI().toURL();

      String classpath = System.getProperty("java.class.path");
      classpath = classpath + System.getProperty("path.separator") + execPathFile.toString();
      System.setProperty("java.class.path", classpath);

      return 0;
    } catch (Exception e) {
      System.out.println("Exec Error = " + e);
    }

    return 1;
  }

  public static Class<?>[] loadFromFile(String[] paths) {
    Class<?>[] handleObject = new Class<?>[paths.length];

    for (int i = 0; i < paths.length; i++) {
      System.out.println("Path provided " + paths[i]);

      try {
        JavaCompiler compiler = ToolProvider.getSystemJavaCompiler();
        DiagnosticCollector<JavaFileObject> ds = new DiagnosticCollector<>();
        StandardJavaFileManager mgr = compiler.getStandardFileManager(ds, null, null);

        Iterable<String> classOutputPath = Arrays.asList(new String[] { "-d", System.getenv("LOADER_SCRIPT_PATH") });

        File pathFile = new File(paths[i]);
        Iterable<? extends JavaFileObject> sources = mgr.getJavaFileObjectsFromFiles(Arrays.asList(pathFile));
        JavaCompiler.CompilationTask task = compiler.getTask(null, mgr, ds, classOutputPath, null, sources);
        Boolean call = task.call(); // main method to compile the file into class

        if (call) {
          System.out.println("Compilation Successful");

          Path path = Paths.get(pathFile.getCanonicalPath());
          String classname = path.getFileName().toString().split(".java")[0];

          for (String curExecPath : executionPath) {
            try {
              File execPathFile = new File(curExecPath);
              URLClassLoader clsLoader = new URLClassLoader(new URL[] { execPathFile.toURI().toURL() });

              // handleObject.addClass(classname, clsLoader.loadClass(classname));
              handleObject[i] = clsLoader.loadClass(classname);
              clsLoader.close();

              System.out.println(i + " -> " + classname + " loaded");

              // handleArray.addClass(classname, Class.forName(classname));
              System.out.println("Class Loading Successful");
              break;

            } catch (Exception e) {
            }
          }
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

    return handleObject;
  }

  public static Class<?>[] load_from_package(String path) {

    if (path.endsWith(".class")) {
      Class<?>[] handleObject = new Class<?>[1];
      System.out.println("bootstrap load from package " + path);

      for (String curExecPath : executionPath) {
        try {
          File pathFile = new File(path);
          Path canonical = Paths.get(pathFile.getCanonicalPath());
          String classname = canonical.getFileName().toString().split(".class")[0];

          File execPathFile = new File(curExecPath);
          URLClassLoader clsLoader = new URLClassLoader(new URL[] { execPathFile.toURI().toURL() });
          Class<?> c = clsLoader.loadClass(classname);

          handleObject[0] = c;
          clsLoader.close();
          break;
        } catch (Exception e) {
          System.out.println("EXEPTION " + e);
        }
      }

      return handleObject;
    } else if (path.endsWith(".jar")) {

      try {
        for (String curExecPath : executionPath) {
          ArrayList<Class<?>> handleList = new ArrayList<Class<?>>();

          Path curJarPath = Paths.get(curExecPath, path);
          JarFile jarFile = new JarFile(curJarPath.toString());
          Enumeration<JarEntry> e = jarFile.entries();

          Path jpath = Paths.get("jar:file:", curExecPath, path);
          String jarPath = jpath.toString() + "!/";

          Path epath = Paths.get(curExecPath, path);
          executionPath.add(epath.toString());

          URLClassLoader clsLoader = new URLClassLoader(new URL[] { new URL(jarPath) });

          while (e.hasMoreElements()) {

            JarEntry je = e.nextElement();
            if (je.getName().endsWith(".class")) {

              String className = je.getName().substring(0, je.getName().length() - 6);
              className = className.replace(File.separatorChar, '.');
              try {
                Class<?> c = clsLoader.loadClass(className);

                if (c != null) {
                  System.out.println("Got CLass " + c.getName());
                  handleList.add(c);

                }
              } catch (Exception ex) {
                System.out.println(ex);
              }

            }
          }

          clsLoader.close();

          Class<?>[] rtnClsArr = new Class<?>[handleList.size()];
          rtnClsArr = handleList.toArray(rtnClsArr);
          jarFile.close();

          return rtnClsArr;
        }
      } catch (Exception e) {
        System.out.println("EXCEPTION " + e);
      }
    }

    return null;
  }

  public static String getSignature(Method m) {
    String sig;
    try {
      Field gSig = Method.class.getDeclaredField("signature");
      gSig.setAccessible(true);
      sig = (String) gSig.get(m);
      if (sig != null)
        return sig;
    } catch (IllegalAccessException | NoSuchFieldException e) {
      e.printStackTrace();
    }

    StringBuilder sb = new StringBuilder("(");

    for (Class<?> c : m.getParameterTypes())
      sb.append((sig = Array.newInstance(c, 0).toString()).substring(1, sig.indexOf('@')));

    return sb.append(')').append(m.getReturnType() == void.class ? "V"
        : (sig = Array.newInstance(m.getReturnType(), 0).toString()).substring(1, sig.indexOf('@'))).toString();
  }

  public static String get_Field_Type(Class<?> cls, String key) {
    System.out.println("Getting field type bootstrap for " + key);
    String valType = "";

    try {
      Field f = cls.getField(key);
      valType = f.getType().getName();
    } catch (Exception e) {
      System.out.println("Finding field error" + e);
    }

    return valType;
  }

  public static String get_static_invoke_return_type(Class<?> cls, String key) {
    String rtnType = "";

    try {
      System.out.println("ClassName: " + cls.getName() + " " + key);

      Method[] methods = cls.getDeclaredMethods();

      for (Method method : methods) {
        System.out.println("Name of the method: " + method.getName());
      }

      Class<?>[] cArg = new Class[1];
      cArg[0] = String.class;

      Method m = cls.getDeclaredMethod(key, cArg);
      System.out.println("OUR: " + m.getReturnType().getName());

    } catch (Exception e) {
      e.printStackTrace();
    }

    return rtnType;
  }

  public static String java_bootstrap_get_class_name(Class<?> cls) {
    // Working test for getting function name and details

    Constructor<?>[] constructors = cls.getDeclaredConstructors();
    for (Constructor<?> cnstr : constructors) {
      System.out.println("Name of the constructor: " + cnstr.getName());
    }

    Field[] fields = cls.getFields();
    for (Field f : fields) {
      System.out.println("Name of the fiekd: " + f.getName());
    }

    Method[] methods = cls.getDeclaredMethods();
    for (Method method : methods) {
      System.out.println("Name of the method: " + method.getName());
      System.out.println("Signature " + getSignature(method));

    }

    return cls.getName();
  }

  // public static void DiscoverData(String classname) {

  // Method[] methods = hClass.getDeclaredMethods();

  // for (Method method : methods) {
  // System.out.println("Name of the method: " + method.getName());

  // Class<?>[] parameters = method.getParameterTypes();
  // if (parameters.length == 0)
  // System.out.println("\tparameter: none");
  // for (Class<?> parameter : parameters) {
  // System.out.println("\tparameter: " + parameter.getSimpleName());
  // }
  // System.out.println("\tReturn Type: " + method.getReturnType() + "\n");

  // }
  // }
}
