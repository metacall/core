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
using System.Collections.Immutable;

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
        static Loader()
        {
            AssemblyLoadContext.Default.Resolving += Context_Resolving;
            Init();
        }
        
        public static void Init()
        {
            loader = new Loader();
        }

        public unsafe static bool LoadFromPointer(string[] source)
        {
            if (loader == null)
            {
                loader = new Loader();
            }

            return loader.LoadFromSourceFunctions(source);
        }

        public static bool Load(string source)
        {
            if (loader == null)
            {
                loader = new Loader();
            }

            return loader.LoadFromSourceFunctions(new string[] { source });
        }

        public static bool Load(string[] files)
        {
            if (loader == null)
            {
                loader = new Loader();
            }

            return loader.LoadFromSourceFunctions(files.Select(x => System.IO.File.ReadAllText(x)).ToArray());
        }

        public ReflectFunction[] Functions()
        {
            return this.functions.Select(x => x.Value.GetReflectFunction()).ToArray();
        }

        public static ReflectFunction[] GetFunctionsInternal()
        {
            return loader.Functions();
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

        public unsafe static bool LoadFilesC([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]IntPtr[] source, long size)
        {
            return Load(source.Select(x => Marshal.PtrToStringAnsi(x)).ToArray());
        }

        public unsafe static bool LoadFilesW([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]IntPtr[] source, long size)
        {
            return Load(source.Select(x => Marshal.PtrToStringUni(x)).ToArray());
        }

        public static bool LoadAssemblyC([MarshalAs(UnmanagedType.LPStr)]string assemblyFile)
        {
            return LoadFromAssembly(assemblyFile);
        }

        public static bool LoadAssemblyW([MarshalAs(UnmanagedType.LPWStr)]string assemblyFile)
        {
            return LoadFromAssembly(assemblyFile);
        }

        public static bool LoadSourceC([MarshalAs(UnmanagedType.LPStr)]string source)
        {
            return Load(source);
        }

        public static bool LoadSourceW([MarshalAs(UnmanagedType.LPWStr)]string source)
        {
            return Load(source);
        }

        public unsafe static IntPtr ExecuteC([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string function)
        {
            return ExecuteFunction(function);
        }

        public unsafe static IntPtr ExecuteW([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPWStr)] string function)
        {
            return ExecuteFunction(function);
        }

        public unsafe static void DestroyExecutionResult(ExecutionResult* executionResult)
        {
            if (executionResult->ptr != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(executionResult->ptr);
            }

            Marshal.FreeHGlobal((IntPtr)executionResult);
        }

        public unsafe static IntPtr ExecuteWithParamsC([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string function,
         [MarshalAs(UnmanagedType.LPArray, SizeConst = 10)]   Parameters[] parameters)
        {
            if (loader == null)
            {
                Console.WriteLine("Loader is null");
                return (IntPtr)CreateExecutionResult(true, type_primitive_id.TYPE_PTR);
            }
            else
            {
                Console.WriteLine("Executing " + function);
                return (IntPtr)loader.Execute(function, parameters);
            }
        }

        public unsafe static IntPtr ExecuteWithParamsW([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPWStr)] string function,
         [MarshalAs(UnmanagedType.LPArray, SizeConst = 10)]   Parameters[] parameters)
        {
            if (loader == null)
            {
                Console.WriteLine("Loader is null");
                return (IntPtr)CreateExecutionResult(true, type_primitive_id.TYPE_PTR);
            }
            else
            {
                Console.WriteLine("Executing " + function);
                return (IntPtr)loader.Execute(function, parameters);
            }
        }

        public unsafe static IntPtr ExecuteFunction(string function)
        {
            if (loader == null)
            {
                Console.WriteLine("Loader is null");
                return (IntPtr)CreateExecutionResult(true, type_primitive_id.TYPE_PTR);
            }
            else
            {
                Console.WriteLine("Executing " + function);
                return (IntPtr)loader.Execute(function);
            }
        }

        private Dictionary<string, FunctionContainer> functions = new Dictionary<string, FunctionContainer>();

        public Loader()
        {
        }

        public bool LoadFromSourceFunctions(string[] source)
        {
            Assembly assembly = null;

            SyntaxTree[] syntaxTrees = source.Select(x => CSharpSyntaxTree.ParseText(x)).ToArray();

            string assemblyName = Path.GetRandomFileName();

            MetadataReference[] references = new MetadataReference[]
{
    MetadataReference.CreateFromFile(typeof(object).GetTypeInfo().Assembly.Location),
    MetadataReference.CreateFromFile(typeof(Enumerable).GetTypeInfo().Assembly.Location)
};

            CSharpCompilation compilation = CSharpCompilation.Create(
                assemblyName,
                syntaxTrees: syntaxTrees,
                references: references,
                options: new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));

            using (var ms = new MemoryStream())
            {
                EmitResult result = compilation.Emit(ms);

                if (!result.Success)
                {
                    IEnumerable<Diagnostic> failures = result.Diagnostics.Where(diagnostic =>

                        diagnostic.Severity == DiagnosticSeverity.Error);

                    foreach (Diagnostic diagnostic in failures)
                    {
                        Console.Error.WriteLine("{ 0}: {1}", diagnostic.Id, diagnostic.GetMessage());
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

        protected static List<string> paths = new List<string>();

        public static bool LoadFromAssembly(string assemblyFile)
        {
            AssemblyLoadContext context = AssemblyLoadContext.Default;
            Assembly asm = null;

            string path = System.IO.Path.GetDirectoryName(assemblyFile);

            if (!paths.Contains(path))
            {
                paths.Add(path);
            }

            try
            {
                asm = context.LoadFromAssemblyPath(assemblyFile);
            }
            catch (Exception)
            {
            }
            if (asm == null)
            {
                try
                {
                    asm = context.LoadFromAssemblyName(new AssemblyName(System.IO.Path.GetFileNameWithoutExtension(assemblyFile)));
                }
                catch (Exception)
                {
                }
            }


            if (asm != null)
            {
                if (loader == null)
                {
                    loader = new Loader();
                }

                loader.LoadFunctions(asm);
                return true;
            }

            return false;
        }

        private static Assembly Context_Resolving(AssemblyLoadContext context, AssemblyName name)
        {
            Assembly asm = null;

            foreach (var path in paths)
            {
                try
                {
                    asm = context.LoadFromAssemblyPath(path + "\\" + name.Name + ".dll");
                    if (asm != null)
                    {
                        return asm;
                    }
                }
                catch (Exception ex)
                {
                }
            }

            return asm;
        }

        public static bool LoadFromAssemblyC([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPWStr)] string assemblyFile)
        {
            return LoadFromAssembly(assemblyFile);
        }

        public static bool LoadFromAssemblyW([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string assemblyFile)
        {
            return LoadFromAssembly(assemblyFile);
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

        public unsafe ExecutionResult* Execute(string function, Parameters[] parameters)
        {
            var objs = new object[parameters.Length];

            var con = this.functions[function];

            for (int i = 0; i < con.Parameters.Length; i++)
            {
                objs[i] = MetacallDef.GetValue(parameters[i].type, parameters[i].ptr);
            }

            var result = con.Method.Invoke(null, objs.Take(con.Parameters.Length).ToArray());

            if (result == null)
            {
                return CreateExecutionResult(false, MetacallDef.Get(con.RetunType));
            }
            else
            {
                return CreateExecutionResult(false, MetacallDef.Get(con.RetunType), result);
            }
        }

        public unsafe ExecutionResult* Execute(string function)
        {
            var con = this.functions[function];
            try
            {
                var result = con.Method.Invoke(null, null);

                if (result == null)
                {
                    return CreateExecutionResult(false, MetacallDef.Get(con.RetunType));
                }
                else
                {
                    return CreateExecutionResult(false, MetacallDef.Get(con.RetunType), result);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }

            return null;
        }

        public unsafe static ExecutionResult* CreateExecutionResult(bool failed, type_primitive_id type)
        {
            ExecutionResult* er = (ExecutionResult*)Marshal.AllocHGlobal(sizeof(ExecutionResult));
            er->failed = failed;
            er->type = type;
            er->ptr = IntPtr.Zero;
            return er;
        }

        public unsafe static ExecutionResult* CreateExecutionResult(bool failed, type_primitive_id type, object value)
        {
            ExecutionResult* er = CreateExecutionResult(failed, type);

            er->ptr = MetacallDef.GetIntPtr(type, value);

            return er;
        }
    }
}