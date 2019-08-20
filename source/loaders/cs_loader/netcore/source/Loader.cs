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
        [System.Diagnostics.Conditional("DEBUG_FILE")]
        private static void Log(string text)
        {
            // TODO: Expose logs from host and use them here
            var message= $"{DateTime.Now.Ticks}: {text}\n";
            System.IO.File.AppendAllText("cs_loader_log.txt", message);
        }


        #if NETCOREAPP1_0 || NETCOREAPP1_1
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
                        Console.Error.WriteLine(ex.Message);
                    }
                }

                return asm;
            }
        #elif NETCOREAPP2_0 || NETCOREAPP2_1 || NETCOREAPP2_2
            private static Assembly AssemblyResolveEventHandler(object sender, ResolveEventArgs args)
            {
                Assembly asm = null;

                Log("AssemblyResolveEventHandler " + paths.Count.ToString());

                foreach (var path in paths)
                {
                    try
                    {
                        var p = path + "\\" + args.Name + ".dll";

                        Log(p);

                        asm = Assembly.LoadFile(p);

                        if (asm != null)
                        {
                            return asm;
                        }
                        else
                        {
                            // TODO: Write proper error message handling
                            Console.Error.WriteLine("Invalid Assembly.LoadFile: " + p);
                        }
                    }
                    catch (Exception ex)
                    {
                        // TODO: Write proper error message handling
                        Console.Error.WriteLine("Exception when loading the Assembly {0}: {1}", args.Name, ex.Message);
                    }
                }

                return asm;
            }
        #endif

        public static void Main(string[] args)
        {

        }

        private static Loader loader = null;

        static Loader()
        {
            Log("CSLoader static initialization");

            #if NETCOREAPP1_0 || NETCOREAPP1_1
                AssemblyLoadContext.Default.Resolving += Context_Resolving;
            #elif NETCOREAPP2_0 || NETCOREAPP2_1 || NETCOREAPP2_2
                AppDomain.CurrentDomain.AssemblyResolve += new ResolveEventHandler(AssemblyResolveEventHandler);
                AppDomain.CurrentDomain.TypeResolve+= new ResolveEventHandler(AssemblyResolveEventHandler);
            #endif

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
                return (IntPtr)CreateExecutionResult(true, type_primitive_id.TYPE_PTR);
            }
            else
            {
                return (IntPtr)loader.Execute(function, parameters);
            }
        }

        public unsafe static IntPtr ExecuteWithParamsW([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPWStr)] string function,
         [MarshalAs(UnmanagedType.LPArray, SizeConst = 10)]   Parameters[] parameters)
        {
            if (loader == null)
            {
                return (IntPtr)CreateExecutionResult(true, type_primitive_id.TYPE_PTR);
            }
            else
            {
                return (IntPtr)loader.Execute(function, parameters);
            }
        }

        public unsafe static IntPtr ExecuteFunction(string function)
        {
            if (loader == null)
            {
                return (IntPtr)CreateExecutionResult(true, type_primitive_id.TYPE_PTR);
            }
            else
            {
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

            #if NETCOREAPP1_0 || NETCOREAPP1_1
                MetadataReference[] references = new MetadataReference[]
                {
                    MetadataReference.CreateFromFile(typeof(object).GetTypeInfo().Assembly.Location),
                    MetadataReference.CreateFromFile(typeof(Enumerable).GetTypeInfo().Assembly.Location)
                };
            #elif NETCOREAPP2_0 || NETCOREAPP2_1 || NETCOREAPP2_2
                MetadataReference[] references;

                var mainPath = Path.GetDirectoryName( typeof(object).GetTypeInfo().Assembly.Location) + "/";
                var assemblyFiles = System.IO.Directory.GetFiles(mainPath, "*.dll");

                assemblyFiles = assemblyFiles.Concat(System.IO.Directory.GetFiles(AppDomain.CurrentDomain.BaseDirectory, "*.dll")).Distinct().ToArray();

                references = assemblyFiles.Select(x => MetadataReference.CreateFromFile(x)).ToArray();
            #endif

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
                        // TODO: Write proper error message handling
                        Console.Error.WriteLine("{0}: {1}", diagnostic.Id, diagnostic.GetMessage());
                    }
                }
                else
                {
                    ms.Seek(0, SeekOrigin.Begin);

                    #if NETCOREAPP1_0 || NETCOREAPP1_1
                        AssemblyLoadContext context = AssemblyLoadContext.Default;
                        assembly = context.LoadFromStream(ms);
                    #elif NETCOREAPP2_0 || NETCOREAPP2_1 || NETCOREAPP2_2
                        assembly = Assembly.Load(ms.ToArray());
                    #endif
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
            #if NETCOREAPP1_0 || NETCOREAPP1_1
                AssemblyLoadContext context = AssemblyLoadContext.Default;
            #endif
            Assembly asm = null;

            string path = System.IO.Path.GetDirectoryName(assemblyFile);

            if (!paths.Contains(path))
            {
                paths.Add(path);
            }

            try
            {
                #if NETCOREAPP1_0 || NETCOREAPP1_1
                    asm = context.LoadFromAssemblyPath(assemblyFile);
                #elif NETCOREAPP2_0 || NETCOREAPP2_1 || NETCOREAPP2_2
                    asm = Assembly.LoadFile(assemblyFile);
                #endif
            }
            catch (Exception)
            {
            }
            if (asm == null)
            {
                try
                {
                    #if NETCOREAPP1_0 || NETCOREAPP1_1
                        asm = context.LoadFromAssemblyName(new AssemblyName(System.IO.Path.GetFileNameWithoutExtension(assemblyFile)));
                    #elif NETCOREAPP2_0 || NETCOREAPP2_1 || NETCOREAPP2_2
                        asm = Assembly.Load(new AssemblyName(System.IO.Path.GetFileNameWithoutExtension(assemblyFile)));
                    #endif
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
                // TODO: Write proper error message handling
                Console.Error.WriteLine("Error executing function {0}: {1}", function, ex.Message);
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