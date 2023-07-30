import java.io.File;
import java.io.FileWriter;

import javax.tools.*;
import java.util.*;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.net.URL;
import java.net.URLClassLoader;
import java.lang.reflect.Modifier;
import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

import java.nio.file.Path;
import java.nio.file.Paths;

public class bootstrap {
  private static Set<String> executionPath = new HashSet<String>();

  public static Class<?> FindClass(String name) {
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
      try {
        for (String curExecPath : executionPath) {
          JavaCompiler compiler = ToolProvider.getSystemJavaCompiler();
          DiagnosticCollector<JavaFileObject> ds = new DiagnosticCollector<>();
          StandardJavaFileManager mgr = compiler.getStandardFileManager(ds, null, null);

          Iterable<String> classOutputPath = Arrays.asList(new String[] { "-d", curExecPath });

          File pathFile = new File(curExecPath, paths[i]);
          Iterable<? extends JavaFileObject> sources = mgr.getJavaFileObjectsFromFiles(Arrays.asList(pathFile));
          JavaCompiler.CompilationTask task = compiler.getTask(null, mgr, ds, classOutputPath, null, sources);
          Boolean call = task.call(); // main method to compile the file into class

          if (call) {
            System.out.println("Compilation Successful");

            Path path = Paths.get(pathFile.getCanonicalPath());
            String classname = path.getFileName().toString().split(".java")[0];

            File execPathFile = new File(curExecPath);
            URLClassLoader clsLoader = new URLClassLoader(new URL[] { execPathFile.toURI().toURL() });

            handleObject[i] = clsLoader.loadClass(classname);
            clsLoader.close();

            System.out.println(classname + " Class Loading Successful");
            break;
          } else {
            System.out.println("Compilation Failed");
          }

          for (Diagnostic<? extends JavaFileObject> d : ds.getDiagnostics()) { // diagnostic error printing
            System.out.format("DIAGNOSTIC Line: %d, %s in %s\n", d.getLineNumber(), d.getMessage(null),
                d.getSource().getName());
          }

          mgr.close();
        }
      } catch (Exception e) {
        System.err.println("Load Function" + e);
      }
    }

    return handleObject;
  }

  public static Class<?>[] load_from_package(String path) {

    if (path.endsWith(".class")) {
      Class<?>[] handleObject = new Class<?>[1];

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
          // TODO: Implement better error handling
          System.out.println("EXEPTION " + e);
        }
      }

      return handleObject;
    } else if (path.endsWith(".jar")) {

      try {
        for (String curExecPath : executionPath) {
          ArrayList<Class<?>> handleList = new ArrayList<Class<?>>();

          Path curJarPath = Paths.get(curExecPath, path);
          File f = new File(curJarPath.toString());

          if (f.exists() && !f.isDirectory()) {
            JarFile jarFile = new JarFile(curJarPath.toString());
            Enumeration<JarEntry> e = jarFile.entries();

            Path jpath = Paths.get(curExecPath, path);
            String jarPath = "jar:file:" + jpath.toString().replace("\\", "/") + "!/";

            Path epath = Paths.get(curExecPath, path);
            executionPath.add(epath.toString());

            URLClassLoader clsLoader = new URLClassLoader(new URL[] { new URL(jarPath) });

            while (e.hasMoreElements()) {

              JarEntry je = e.nextElement();
              if (je.getName().endsWith(".class")) {

                String className = je.getName().substring(0, je.getName().length() - 6);
                className = className.replace('/', '.');
                try {
                  Class<?> c = clsLoader.loadClass(className);

                  if (c != null) {
                    handleList.add(c);
                  }
                } catch (Exception ex) {
                  // TODO: Implement better error handling
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
        }
      } catch (Exception e) {
        // TODO: Implement better error handling
        System.out.println("EXCEPTION " + e);
      }
    }

    return null;
  }

  public static Class<?>[] load_from_memory(String name, String buffer) {
    try {
      Path tempFile = Paths.get(System.getenv("LOADER_SCRIPT_PATH"), "memoryTest.java");

      FileWriter myWriter = new FileWriter(tempFile.toString());
      myWriter.write(buffer);
      myWriter.close();

      return loadFromFile(new String[] { tempFile.toString() });
    } catch (Exception e) {
      System.err.println("Creating File error " + e);
    }
    return new Class<?>[] {};
  }

  private static String[] getTypeSignature(Class<?> t) {
    String name = t.getName();
    boolean primitive = t.isPrimitive();
    String signature = name.replace(".", "/");

    return new String[] { name, primitive ? "L" + signature + ";" : signature };
  }

  // Holds a mapping from Java type names to native type codes
  private static final Map<Class<?>, String> PRIMITIVE_TO_SIGNATURE;
  static {
    PRIMITIVE_TO_SIGNATURE = new HashMap<Class<?>, String>(9);
    PRIMITIVE_TO_SIGNATURE.put(byte.class, "B");
    PRIMITIVE_TO_SIGNATURE.put(char.class, "C");
    PRIMITIVE_TO_SIGNATURE.put(short.class, "S");
    PRIMITIVE_TO_SIGNATURE.put(int.class, "I");
    PRIMITIVE_TO_SIGNATURE.put(long.class, "J");
    PRIMITIVE_TO_SIGNATURE.put(float.class, "F");
    PRIMITIVE_TO_SIGNATURE.put(double.class, "D");
    PRIMITIVE_TO_SIGNATURE.put(void.class, "V");
    PRIMITIVE_TO_SIGNATURE.put(boolean.class, "Z");
  }

  // Returns the internal name of {@code clazz} (also known as the descriptor)
  private static String getSignature(Class<?> clazz) {
    String primitiveSignature = PRIMITIVE_TO_SIGNATURE.get(clazz);
    if (primitiveSignature != null) {
      return primitiveSignature;
    } else if (clazz.isArray()) {
      return "[" + getSignature(clazz.getComponentType());
    } else {
      return "L" + clazz.getName().replace('.', '/') + ";";
    }
  }

  public static String getSignature(Method m) {
    StringBuilder result = new StringBuilder();
    result.append('(');
    Class<?>[] parameterTypes = m.getParameterTypes();
    for (Class<?> parameterType : parameterTypes) {
      result.append(getSignature(parameterType));
    }
    result.append(')');
    result.append(getSignature(m.getReturnType()));
    return result.toString();
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
    /*
    Constructor<?>[] constructors = cls.getDeclaredConstructors();
    for (Constructor<?> cnstr : constructors) {
      System.out.println("Name of the constructor: " + cnstr.getName());
    }

    Field[] fields = cls.getFields();
    for (Field f : fields) {
      System.out.println("Name of the field: " + f.getName());
    }

    Method[] methods = cls.getDeclaredMethods();
    for (Method method : methods) {
      System.out.println("Name of the method: " + method.getName());
      System.out.println("Signature " + getSignature(method));

    }
    */

    return cls.getName();
  }

  public static String getModifierType(int mod) {
    if (Modifier.isPublic(mod))
      return "public";
    if (Modifier.isPrivate(mod))
      return "private";
    if (Modifier.isProtected(mod))
      return "protected";

    return "";
  }

  public static Field[] java_bootstrap_discover_fields(Class<?> cls) {
    Field[] fields = cls.getFields();
    return fields;
  }

  public static String[] java_bootstrap_discover_fields_details(Field f) {
    String fName = f.getName();
    String[] fTypeNameSig = getTypeSignature(f.getType());
    String fTypeName = fTypeNameSig[0];
    String fSignature = fTypeNameSig[1];
    int fModifier = f.getModifiers();
    String fVisibility = getModifierType(fModifier);
    String fStatic = Modifier.isStatic(fModifier) ? "static" : "nonstatic";

    return new String[] { fName, fTypeName, fVisibility, fStatic, fSignature };
  }

  public static int java_bootstrap_discover_method_args_size(Method m) {
    return m.getParameterCount();
  }

  public static Method[] java_bootstrap_discover_methods(Class<?> cls) {
    Method[] methods = cls.getMethods();
    return methods;
  }

  public static Constructor<?>[] java_bootstrap_discover_constructors(Class<?> cls) {
    Constructor<?>[] constructors = cls.getDeclaredConstructors();
    return constructors;
  }

  public static String[] java_bootstrap_discover_method_details(Method m) {
    String mName = m.getName();
    String[] mRetTypeNameSig = getTypeSignature(m.getReturnType());
    String mRetTypeName = mRetTypeNameSig[0];
    String mRetTypeSig = mRetTypeNameSig[1];
    int mModifier = m.getModifiers();
    String mVisibility = getModifierType(mModifier);
    String mStatic = Modifier.isStatic(mModifier) ? "static" : "nonstatic";
    String mSignature = getSignature(m).replace(".", "/");

    return new String[] { mName, mRetTypeName, mRetTypeSig, mVisibility, mStatic, mSignature };
  }

  public static String[][] java_bootstrap_discover_method_parameters(Method m) {
    String[][] parameterList = new String[java_bootstrap_discover_method_args_size(m)][2];
    Class<?>[] parameters = m.getParameterTypes();

    int i = 0;
    for (Class<?> parameter : parameters) {
      parameterList[i++] = getTypeSignature(parameter);
    }

    return parameterList;
  }
}
