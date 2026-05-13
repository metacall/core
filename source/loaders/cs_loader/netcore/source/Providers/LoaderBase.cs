using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Linq;
using System.Reflection;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis;
using System.IO;
using System.Reflection.Metadata;
using Microsoft.CodeAnalysis.Emit;
using static CSLoader.MetacallDef;
using CSLoader.Contracts;
using System.Runtime.InteropServices;

namespace CSLoader.Providers
{
    public class CollectibleAssemblyLoadContext : System.Runtime.Loader.AssemblyLoadContext, IDisposable
    {
        public CollectibleAssemblyLoadContext() : base(true)
        {

        }

        protected override Assembly Load(AssemblyName assemblyName)
        {
            return null;
        }

        public void Dispose()
        {
            Unload();
        }
    }

    public abstract class LoaderBase : ILoader
    {
        protected readonly ILog log;
        protected readonly List<string> paths = new List<string>();

protected LoaderBase(ILog log)
        {
            this.log = log;
        }

        ~LoaderBase()
        {
            paths.Clear();
        }

        public bool ExecutionPath(string path)
        {
            if (!paths.Contains(path))
            {
                log.Info("Loading execution path: " + path);
                paths.Add(path);
                return true;
            }

            return false;
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

        protected abstract Assembly MakeAssembly(MemoryStream stream, string scriptHandle);

        private static bool IsFullPath(string path)
        {
            // https://stackoverflow.com/a/47569899
            if (string.IsNullOrWhiteSpace(path) || path.IndexOfAny(Path.GetInvalidPathChars()) != -1 || !Path.IsPathRooted(path))
                return false;

            string pathRoot = Path.GetPathRoot(path);
            if (pathRoot.Length <= 2 && pathRoot != "/") // Accepts X:\ and \\UNC\PATH, rejects empty string, \ and X:, but accepts / to support Linux
                return false;

            if (pathRoot[0] != '\\' || pathRoot[1] != '\\')
                return true; // Rooted and not a UNC path

            return pathRoot.Trim('\\').IndexOf('\\') != -1; // A UNC server name without a share name (e.g "\\NAME" or "\\NAME\") is invalid
        }

        private string LoadFromFileFunctionsRelative(string file)
        {
            foreach (var path in paths)
            {
                try
                {
                    return System.IO.File.ReadAllText(Path.Combine(path, file));
                }
                catch
                {
                    continue;
                }
            }

            throw new Exception("File " + file + " not found");
        }

        public bool LoadFromFileFunctions(string[] files)
        {
            List<string> sources = new List<string>();

            foreach (var file in files)
            {
                try
                {
                    sources.Add(IsFullPath(file) ? System.IO.File.ReadAllText(file) : LoadFromFileFunctionsRelative(file));
                }
                catch (Exception ex)
                {
                    this.log.Error("CSLoader loading error: " + ex.Message);
                    return false;
                }
            }

            if (files.Length != sources.Count)
            {
                this.log.Error("Not all files could be loaded (" + sources.Count + "/" + files.Length + ")");
                return false;
            }

            return LoadFromSourceFunctions(sources.ToArray()) != null ;
        }

        private int PrintDiagnostics(ImmutableArray<Diagnostic> diagnostics)
        {
            int errorCount = 0;

            foreach (Diagnostic diagnostic in diagnostics)
            {
                if (diagnostic.IsWarningAsError || diagnostic.Severity == DiagnosticSeverity.Error)
                {
                    this.log.Error("CSLoader compilation error: " + diagnostic.ToString());
                    ++errorCount;
                }
                else
                {
                    this.log.Error("CSLoader compilation warning: " + diagnostic.ToString());
                }
            }

            return errorCount;
        }

		// existing contract
        public string LoadFromSourceFunctions(string[] source)
        {
			string scriptHandle = Guid.NewGuid().ToString();
			bool success =  LoadFromSourceFunctions(source, scriptHandle);
			return success ? scriptHandle : null;
		}
        public bool LoadFromSourceFunctions(string[] source, string scriptHandle)
        {
            Assembly assembly = null;

            CSharpParseOptions parseOptions = new CSharpParseOptions(kind: SourceCodeKind.Regular, languageVersion: LanguageVersion.Latest);

            SyntaxTree[] syntaxTrees = source.Select(x => CSharpSyntaxTree.ParseText(x, parseOptions)).ToArray();

			bool hasTopLevelStatements = syntaxTrees.Any(tree => tree.GetRoot().DescendantNodes().OfType<Microsoft.CodeAnalysis.CSharp.Syntax.GlobalStatementSyntax>().Any());

			var outputKind = hasTopLevelStatements ? OutputKind.ConsoleApplication : OutputKind.DynamicallyLinkedLibrary;

			string assemblyName = Path.GetRandomFileName();

            MetadataReference[] references;

            var mainPath = Path.GetDirectoryName(typeof(object).GetTypeInfo().Assembly.Location) + Path.DirectorySeparatorChar;
            var assemblyFiles = System.IO.Directory.GetFiles(mainPath, "*.dll");
			// filter to managed assemblies only
			// This is necessary to avoid loading native assemblies which would cause MetadataReference.CreateFromFile to throw an exception
			assemblyFiles = assemblyFiles.Where(x =>
			{
				try
				{
					System.Reflection.AssemblyName.GetAssemblyName(x);
					return true;
				}
				catch
				{
					return false;
				}
			}).ToArray();

			assemblyFiles = assemblyFiles.Concat(this.AdditionalLibs()).Distinct().ToArray();

            // Console exists in both System.Console and System.Private.CoreLib in NetCore 1.x
            // So it must be removed in order to avoid name collision
            #if NETCOREAPP1_0 || NETCOREAPP1_1 || NETCOREAPP1_2
                assemblyFiles = assemblyFiles.Where(asm => !asm.Contains("System.Console")).ToArray();
            #endif

            references = assemblyFiles.Select(x =>
            {
	            try
	            {
					return (MetadataReference)MetadataReference.CreateFromFile(x);
	            }
	            catch
	            {
		            return null;
	            }
            })
	            .Where(r => r != null)
	            .ToArray();

            CSharpCompilation compilation = CSharpCompilation.Create(
                assemblyName,
                syntaxTrees: syntaxTrees,
                references: references,
                options: new CSharpCompilationOptions(
                    outputKind,
                    #if DEBUG
                    optimizationLevel: OptimizationLevel.Debug,
                    #else
                    optimizationLevel: OptimizationLevel.Release,
                    #endif
                    concurrentBuild: true
                )
            );

            if (PrintDiagnostics(compilation.GetDiagnostics()) > 0)
            {
                return false;
            }

            if (PrintDiagnostics(compilation.GetDeclarationDiagnostics()) > 0)
            {
                return false;
            }

            using (var ms = new MemoryStream())
            {
                EmitResult result = compilation.Emit(ms);

                if (!result.Success)
                {
                    PrintDiagnostics(result.Diagnostics);

                    ms.Dispose();

                    return false;
                }
                else
                {
                    ms.Seek(0, SeekOrigin.Begin);

                    assembly = this.MakeAssembly(ms, scriptHandle);

                    this.LoadFunctions(assembly, scriptHandle);

                    ms.Dispose();

                    return true;
                }
            }
        }

        public void LoadFunctions(Assembly assembly, string scriptHandle)
        {
			this.log.Info(" >>> NEW C# LOADER IS ACTUALLY RUNNING! <<<");
			try
			{
				Type[] types = assembly.GetTypes();
				this.log.Info($"[CSLoader] Found {types.Length} types in assembly {assembly.FullName}");
				foreach (Type type in types)
				{
					this.log.Info($"[CSLoader] Inspecting Type: {type.Name}");

					// Handle top-level function class
					bool isTopLevelClass = (type.Name == "<Program>$" || type.Name == "Program") && type.GetMethod("<Main>$", BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.DeclaredOnly) != null;


					if (isTopLevelClass)
					{
						const string prefix = "<<Main>$>g__";
						var allMethods =
							type.GetMethods(BindingFlags.NonPublic | BindingFlags.Static | BindingFlags.DeclaredOnly);
						foreach (var m in allMethods)
						{
							if (m.Name.StartsWith(prefix))
							{
								int pipe = m.Name.IndexOf('|');
								if (pipe > 0)
								{
									string funcName = m.Name.Substring(prefix.Length, pipe - prefix.Length);
									this.functions[funcName] = new FunctionContainer(m, null, scriptHandle, funcName);
									this.log.Info($"[CSLoader] Loading top-level function: {funcName}");
								}
							}
						}
						continue;
					}

					if (type.Name.StartsWith("<") && type.Name != "<Program>$") continue;

					bool isStaticClass = type.IsAbstract && type.IsSealed;

					if (type.IsClass && (!type.IsAbstract || isStaticClass))
					{
						object classInstance = null;
						if (!isStaticClass)
						{
							try
							{
								classInstance = Activator.CreateInstance(type);
								this.log.Info($"[CSLoader] Instantiated: {type.Name}");
							}
							catch (Exception ex)
							{
								this.log.Info($"[CSLoader] Could not instantiate '{type.Name}': {ex.Message}");
							}
						}
						
						var methods = type.GetMethods((BindingFlags.Public | BindingFlags.Instance |
						                               BindingFlags.Static | BindingFlags.DeclaredOnly));
						this.log.Info($"[CSLoader] Found {methods.Length} methods in {type.Name}");

						foreach (MethodInfo method in methods)
						{
							if (method.DeclaringType == typeof(object)) continue;

							//string exportedName = $"{type.Name}.{method.Name}";
							// Change it back later
							string exportedName = method.Name;
							object target = method.IsStatic ? null : classInstance;


							var con = new FunctionContainer(method, target, scriptHandle);

							this.log.Info($"[CSLoader] Loading function: {exportedName} | Static: {method.IsStatic}");

							
							this.functions[exportedName] = con;
						}
					}
				}
			}
			catch (Exception ex)
			{
				this.log.Info($"[SCREAMING DEBUG] FATAL CRASH IN C#: {ex.Message}");
			}
			

            GC.Collect();
        }

        public string LoadFromAssembly(string assemblyFile)
        {
            Assembly asm = null;

            string path = System.IO.Path.GetDirectoryName(assemblyFile);

            if (!paths.Contains(path))
            {
                paths.Add(path);
            }


            try
            {
                asm = this.LoadFile(assemblyFile, assemblyFile);
            }
            catch (Exception ex)
            {
                this.log.Error(ex.Message, ex);

                try
                {
                    asm = this.Load(new AssemblyName(System.IO.Path.GetFileNameWithoutExtension(assemblyFile)), assemblyFile);
                }
                catch (Exception exName)
                {
                    this.log.Error(exName.Message, exName);
                    return null;
                }
            }

            if (asm != null)
            {
                this.LoadFunctions(asm, assemblyFile);
				return assemblyFile;
            }

			return null;
        }

        public unsafe ExecutionResult* Execute(string function, Parameters[] parameters)
        {
            var objs = new object[parameters.Length];

            var con = this.functions[function];

            for (int i = 0; i < con.Parameters.Length; i++)
            {
                objs[i] = MetacallDef.GetValue(parameters[i].type, parameters[i].ptr);
            }

            var result = con.Method.Invoke(con.TargetInstance, objs.Take(con.Parameters.Length).ToArray());

            if (result == null)
            {
                return CreateExecutionResult(false, MetacallDef.Get(con.ReturnType));
            }
            else
            {
                return CreateExecutionResult(false, MetacallDef.Get(con.ReturnType), result);
            }
        }

        public unsafe ExecutionResult* Execute(string function)
        {
            var con = this.functions[function];
            try
            {
                var result = con.Method.Invoke(con.TargetInstance, null);

                if (result == null)
                {
                    return CreateExecutionResult(false, MetacallDef.Get(con.ReturnType));
                }
                else
                {
                    return CreateExecutionResult(false, MetacallDef.Get(con.ReturnType), result);
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

        public abstract void Unload();
        protected abstract Assembly Load(AssemblyName assemblyName, string scriptHandle);
        protected abstract Assembly LoadFile(string assemblyFile, string scriptHandle);
    }
}
