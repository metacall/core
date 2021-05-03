using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis;
using System.IO;
using Microsoft.CodeAnalysis.Emit;
using static CSLoader.MetacallDef;
using CSLoader.Contracts;
using System.Runtime.InteropServices;

namespace CSLoader.Providers
{
    public abstract class LoaderBase : ILoader
    {
        protected readonly ILog log;
        protected readonly List<string> paths = new List<string>();
        protected LoaderBase(ILog log)
        {
            this.log = log;
        }

        public ReflectFunction[] Functions()
        {
            return this.functions.Select(x => x.Value.GetReflectFunction()).ToArray();
        }

        protected Dictionary<string, FunctionContainer> functions = new Dictionary<string, FunctionContainer>();

        protected virtual IEnumerable<string> AdditionalLibs()
        {
            return Array.Empty<string>();
        }

        protected abstract Assembly MakeAssembly(MemoryStream stream);

        public bool LoadFromSourceFunctions(string[] source)
        {
            Assembly assembly = null;

            CSharpParseOptions parseOptions = new CSharpParseOptions(kind: SourceCodeKind.Regular, languageVersion: LanguageVersion.Latest);

            SyntaxTree[] syntaxTrees = source.Select(x => CSharpSyntaxTree.ParseText(x, parseOptions)).ToArray();

            string assemblyName = Path.GetRandomFileName();

            MetadataReference[] references;

            var mainPath = Path.GetDirectoryName(typeof(object).GetTypeInfo().Assembly.Location) + Path.DirectorySeparatorChar;
            var assemblyFiles = System.IO.Directory.GetFiles(mainPath, "*.dll");

            assemblyFiles = assemblyFiles.Concat(this.AdditionalLibs()).Distinct().ToArray();

            // Console exists in both System.Console and System.Private.CoreLib in NetCore 1.x
            // So it must be removed in order to avoid name collision
            #if NETCOREAPP1_0 || NETCOREAPP1_1 || NETCOREAPP1_2
                assemblyFiles = assemblyFiles.Where(asm => !asm.Contains("System.Console")).ToArray();
            #endif

            references = assemblyFiles.Select(x => MetadataReference.CreateFromFile(x)).ToArray();

            CSharpCompilation compilation = CSharpCompilation.Create(
                assemblyName,
                syntaxTrees: syntaxTrees,
                references: references,
                options: new CSharpCompilationOptions(
                    OutputKind.DynamicallyLinkedLibrary,
                    optimizationLevel: OptimizationLevel.Release,
                    concurrentBuild: true
                )
            );

            using (var ms = new MemoryStream())
            {
                EmitResult result = compilation.Emit(ms);

                if (!result.Success)
                {
                    IEnumerable<Diagnostic> failures = result.Diagnostics.Where(diagnostic =>
                        diagnostic.Severity == DiagnosticSeverity.Error);

                    foreach (Diagnostic diagnostic in failures)
                    {
                        this.log.Error("CSLoader compilation error: " + diagnostic.ToString());
                    }

                    return false;
                }
                else
                {
                    ms.Seek(0, SeekOrigin.Begin);

                    assembly = this.MakeAssembly(ms);

                    this.LoadFunctions(assembly);

                    return true;
                }
            }
        }

        public void LoadFunctions(Assembly assembly)
        {
            this.log.Info("CSLoader load functions");

            foreach (var item in assembly.DefinedTypes.SelectMany(x => x.GetMethods()).Where(x => x.IsStatic))
            {
                var con = new FunctionContainer(item);

                this.log.Info("CSLoader loading function: " + item.Name);

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

        public bool LoadFromAssembly(string assemblyFile)
        {
            Assembly asm = null;

            string path = System.IO.Path.GetDirectoryName(assemblyFile);

            if (!paths.Contains(path))
            {
                paths.Add(path);
            }

            try
            {
                asm = this.LoadFile(assemblyFile);
            }
            catch (Exception ex)
            {
                this.log.Error(ex.Message, ex);

                try
                {
                    asm = this.Load(new AssemblyName(System.IO.Path.GetFileNameWithoutExtension(assemblyFile)));
                }
                catch (Exception exName)
                {
                    this.log.Error(exName.Message, exName);
                    return false;
                }
            }

            if (asm != null)
            {
                this.LoadFunctions(asm);
                return true;
            }

            return false;
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
                this.log.Error("Error executing function " + function, ex);
            }

            return null;
        }


        public unsafe ExecutionResult* CreateExecutionResult(bool failed, type_primitive_id type)
        {
            ExecutionResult* er = (ExecutionResult*)Marshal.AllocHGlobal(sizeof(ExecutionResult));
            er->failed = failed;
            er->type = type;
            er->ptr = IntPtr.Zero;
            return er;
        }

        public unsafe ExecutionResult* CreateExecutionResult(bool failed, type_primitive_id type, object value)
        {
            ExecutionResult* er = CreateExecutionResult(failed, type);

            er->ptr = MetacallDef.GetIntPtr(type, value);

            return er;
        }

        protected abstract Assembly Load(AssemblyName assemblyName);
        protected abstract Assembly LoadFile(string assemblyFile);
    }
}