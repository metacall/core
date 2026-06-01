#if NETCOREAPP2_0 || NETCOREAPP2_1 || NETCOREAPP2_2 || NET5_0 || NET7_0

using System;
using System.Collections.Generic;
using System.Reflection;
using System.IO;
using System.Runtime.CompilerServices;
using CSLoader.Contracts;

namespace CSLoader.Providers
{
    public class LoaderV2 : LoaderBase
    {
        public LoaderV2(ILog log) : base(log)
        {
            // This handler is called only when the common language runtime tries to bind to the assembly and fails
            AppDomain.CurrentDomain.AssemblyResolve += new ResolveEventHandler(AssemblyResolveEventHandler);
            AppDomain.CurrentDomain.TypeResolve += new ResolveEventHandler(AssemblyResolveEventHandler);

            loadContext = new Dictionary<string, CollectibleAssemblyLoadContext>();
        }

        private Assembly AssemblyResolveEventHandler(object sender, ResolveEventArgs args)
        {
            Assembly asm = null;
            AssemblyName assemName = args.RequestingAssembly.GetName();

            foreach (var path in paths)
            {
                try
                {
                    var fullPath = Path.Combine(path, assemName.Name + ".dll");

                    asm = Assembly.LoadFile(fullPath);

                    if (asm != null)
                    {
                        log.Info("Assembly loaded: " + fullPath);
                        return asm;
                    }
                    else
                    {
                        log.Error("Invalid Assembly.LoadFile: " + fullPath);
                    }
                }
                catch (FileNotFoundException)
                {
                    continue;
                }
                catch (Exception ex)
                {
                    log.Error(string.Format("Exception when loading the Assembly {0}: {1}", assemName.Name, ex.Message), ex);
                }
            }

            log.Error("Assembly not found: " + assemName.Name);

            return asm;
        }

        protected override IEnumerable<string> AdditionalLibs()
        {
            /*
            IEnumerable<string> libs = System.IO.Directory.GetFiles(AppDomain.CurrentDomain.BaseDirectory, "*.dll");

            foreach (var path in paths)
            {
                libs = libs.Concat(System.IO.Directory.GetFiles(path, "*.dll"));
            }

            return libs; // TODO: Unique
            */
            return System.IO.Directory.GetFiles(AppDomain.CurrentDomain.BaseDirectory, "*.dll");
        }

        protected override Assembly MakeAssembly(MemoryStream stream, string scriptHandle)
        {
	        var alc = new CollectibleAssemblyLoadContext();
			loadContext[scriptHandle] = alc;
            return alc.LoadFromStream(stream);
        }

        protected override Assembly Load(AssemblyName assemblyName, string scriptHandle)
        {
            // Use alc for the current handle if it exits
			if (loadContext.TryGetValue(scriptHandle, out var alc))
            {
                try
                {
                    return alc.LoadFromAssemblyName(assemblyName);
                }
                catch
                {
                    // Fall through to default load
                }
            }
			// fallback to default
            return Assembly.Load(assemblyName);
        }

        protected override Assembly LoadFile(string assemblyFile, string scriptHandle)
        {
	        var alc = new CollectibleAssemblyLoadContext();
	        loadContext[scriptHandle] = alc;
            using (var fs = new FileStream(assemblyFile, FileMode.Open, FileAccess.Read))
            {
                return alc.LoadFromStream(fs);
            }
        }

        public Dictionary<string, int> GetLoadedFunctionsMetadata()
        {
	        var metadata = new Dictionary<string, int>();

	        foreach (var kvp in this.functions)
	        {
		        string functionName = kvp.Key;
		        int parameterCount = kvp.Value.Parameters.Length;

				metadata.Add(functionName, parameterCount);
	        }

	        return metadata;
        }

        // get the count of loaded context for testing
        public int LoadContextCount => loadContext.Count;

		// returns a handle so test can ref it
        public string LoadFromSourceFunctionsWithHandle(string[] source)
        {
			string handle = Guid.NewGuid().ToString();
			LoadFromSourceFunctions(source, handle);
			return handle;
        }
        [MethodImpl(MethodImplOptions.NoInlining)]
        private void UnloadContext(string scriptHandle)
        {
			// Remove all functions in this script
	        var toRemove = new List<string>();

	        foreach (var kvp in functions)
	        {
				if (kvp.Value.ScriptHandle == scriptHandle) toRemove.Add(kvp.Key);
	        }
	        foreach (var key in toRemove)
	        {
		        functions.Remove(key);
	        }

	        if (loadContext.TryGetValue(scriptHandle, out var alc))
	        {
				alc.Unload();
				loadContext.Remove(scriptHandle);
	        }
        }

        public void UnloadScript(string scriptHandle)
        {
			if (!loadContext.ContainsKey(scriptHandle)) return;

			var weakRef = new WeakReference(loadContext[scriptHandle], true);

			// JIT will fully exit before the GC loop
			UnloadContext(scriptHandle);

			for (int i = 0; weakRef.IsAlive && i < 20; i++)
			{
				GC.Collect();
				GC.WaitForPendingFinalizers();
				GC.Collect();
			}

			if (weakRef.IsAlive)
			{
				log.Error($"AssemblyLoadContext for script {scriptHandle} did not unload.");
			}
			else
			{
				log.Info($"AssemblyLoadContext for script {scriptHandle} unloaded successfully.");
			}
		}

        public override void Unload()
        {
			// finally unload all scripts
			var handles = new List<string>(loadContext.Keys);
			foreach (var handle in handles)
			{
				UnloadScript(handle);
			}

			AppDomain.CurrentDomain.AssemblyResolve -= AssemblyResolveEventHandler;
			AppDomain.CurrentDomain.TypeResolve -= AssemblyResolveEventHandler;
		}

        private readonly Dictionary<string, CollectibleAssemblyLoadContext> loadContext = new Dictionary<string, CollectibleAssemblyLoadContext>();
        private WeakReference contextWeakRef;
    }
}
#endif
