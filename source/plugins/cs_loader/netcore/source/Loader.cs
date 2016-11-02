using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis;
using System.IO;
using Microsoft.CodeAnalysis.Emit;
using System.Runtime.Loader;
using System.Runtime.InteropServices;
using static CSLoader.MetacallDef;

namespace CSLoader
{
    public class Loader
    {
        public static void Main(string[] args)
        {
            var src = @"
        public static void DoTheMagic()
        {
            Console.WriteLine(DateTime.Now.ToString());
        }

        public static void DoTheMagic2(int entero,string texto,System.Threading.Tasks.Task task)
        {
            Console.WriteLine(DateTime.Now.ToString());
        }
";
            // Load(src);

        }

        private static Loader loader = null;

        public static void Init()
        {
            loader = new Loader();
        }

        public static bool Load(string source)
        {
            if (loader == null)
            {
                loader = new Loader();
            }

            return loader.LoadFromSourceFunctions(source);
        }
        public ReflectFunction[] Functions()
        {
            return this.functions.Select(x => x.Value.GetReflectFunction()).ToArray();
        }

        public static void GetFunctions(ref int count, IntPtr p)
        {
            var f = loader.Functions();
            count = f.Length;

            foreach (var item in f)
            {
                Marshal.StructureToPtr(item, p, false);
                p += Marshal.SizeOf<ReflectFunction>();
            }
        }


        public static bool LoadW([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPWStr)] string source)
        {
            return Load(source);
        }

        public static bool LoadC([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string source)
        {
            return Load(source);
        }

        public static void ExecuteC([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string function)
        {
            ExecuteFunction(function);
        }

        public static void ExecuteW([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPWStr)] string function)
        {
            ExecuteFunction(function);
        }

        public static bool ExecuteWithParamsC([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string function,
         [MarshalAs(UnmanagedType.LPArray, SizeConst = 10)]   Parameters[] parameters, int size)
        {
            if (loader == null)
            {
                Console.WriteLine("Loader is null");
                return false;
            }
            else
            {
                Console.WriteLine("Executing " + function);
                return loader.Execute(function, parameters.Take(size).ToArray());
            }
        }

        public static bool ExecuteWithParamsW([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPWStr)] string function,
         [MarshalAs(UnmanagedType.LPArray, SizeConst = 10)]   Parameters[] parameters, int size)
        {
            if (loader == null)
            {
                Console.WriteLine("Loader is null");
                return false;
            }
            else
            {
                Console.WriteLine("Executing " + function);
                return loader.Execute(function, parameters.Take(size).ToArray());
            }
        }

        public static bool ExecuteFunction(string function)
        {
            if (loader == null)
            {
                Console.WriteLine("Loader is null");
                return false;
            }
            else
            {
                Console.WriteLine("Executing " + function);
                return loader.Execute(function);
            }
        }

        private Dictionary<string, FunctionContainer> functions = new Dictionary<string, FunctionContainer>();

        public Loader()
        {
        }

        public bool LoadFromSourceFunctions(string source)
        {
            Assembly assembly = null;

            var generatedSource = this.GenerateSource(source);

            SyntaxTree syntaxTree = CSharpSyntaxTree.ParseText(generatedSource);

            string assemblyName = Path.GetRandomFileName();
            MetadataReference[] references = new MetadataReference[]
            {
    MetadataReference.CreateFromFile(typeof(object).GetTypeInfo().Assembly.Location),
    MetadataReference.CreateFromFile(typeof(Enumerable).GetTypeInfo().Assembly.Location)
            };

            CSharpCompilation compilation = CSharpCompilation.Create(
                assemblyName,
                syntaxTrees: new[] { syntaxTree },
                references: references,
                options: new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));


            using (var ms = new MemoryStream())
            {
                EmitResult result = compilation.Emit(ms);

                if (!result.Success)
                {
                    IEnumerable<Diagnostic> failures = result.Diagnostics.Where(diagnostic =>
                        diagnostic.IsWarningAsError ||
                        diagnostic.Severity == DiagnosticSeverity.Error);

                    foreach (Diagnostic diagnostic in failures)
                    {
                        Console.Error.WriteLine("{0}: {1}", diagnostic.Id, diagnostic.GetMessage());
                    }
                }
                else
                {
                    ms.Seek(0, SeekOrigin.Begin);

                    AssemblyLoadContext context = AssemblyLoadContext.Default;
                    assembly = context.LoadFromStream(ms);
                }
            }

            if (assembly != null)
            {
                this.LoadFunctions(assembly);
                return true;
            }
            else
            {
                return false;
            }
        }

        private string GenerateSource(string source)
        {
            var src = @"
    using System;

    namespace BEAST
    {
        public static class BEASTFUNCTIONCONTAINER
        {
           %FUNCS%
        }
    }".Replace("%FUNCS%", source);


            return src;
        }

        public void LoadFunctions(Assembly assembly)
        {
            foreach (var item in assembly.DefinedTypes.SelectMany(x => x.GetMethods()).Where(x => x.IsStatic))
            {
                var con = new FunctionContainer(item);

                if (!this.functions.ContainsKey(item.Name))
                {
                    this.functions.Add(item.Name, con);
                }
                else
                {
                    this.functions[item.Name] = con;
                }
            }

            GC.Collect();
        }

        public bool Execute(string function, Parameters[] parameters)
        {
            var objs = new object[parameters.Length];

            var con = this.functions[function];

            for (int i = 0; i < parameters.Length; i++)
            {
                if (parameters[i].type == type_primitive_id.TYPE_STRING)
                {
                    objs[i] = Marshal.PtrToStringUni(parameters[i].ptr);
                }
                else
                {
                    objs[i] = Marshal.PtrToStringAnsi(parameters[i].ptr);
                }

            }

            con.Method.Invoke(null, objs);
            return true;
        }

        public bool Execute(string function)
        {
            var con = this.functions[function];

            con.Method.Invoke(null, null);
            return true;
            /*
            Type container = assembly.GetType("BEAST.BEASTFUNCTIONCONTAINER");

            List<Container> col = new List<Container>();

            foreach (var item in assembly.DefinedTypes.SelectMany(x => x.GetMethods()).Where(x => x.IsStatic))
            {
                var con = new Container();
                con.FunctionName = item.Name;
                con.RetunType = item.ReturnType.Name ?? "void";
                con.Parameters = item.GetParameters();
                con.Assembly = assembly;
                col.Add(con);
            }

            var f = container.GetTypeInfo().GetMethod(function);

            //f.Invoke(null, null);

            foreach (var item in col)
            {
                Console.WriteLine(item.FunctionName);
                Console.WriteLine(item.RetunType);
                if (item.Parameters != null)
                {
                    foreach (var par in item.Parameters)
                    {
                        Console.WriteLine($"type:{par.ParameterType.Name} name:{par.Name}");
                    }
                }
            }
            */
        }
    }
}