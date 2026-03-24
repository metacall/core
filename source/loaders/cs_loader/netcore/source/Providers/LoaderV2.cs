#if NETCOREAPP2_0 || NETCOREAPP2_1 || NETCOREAPP2_2 || NET5_0 || NET7_0

using System;
using System.Collections.Generic;
using System.Reflection;
using System.IO;
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

            loadContext = new CollectibleAssemblyLoadContext();
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

        protected override Assembly MakeAssembly(MemoryStream stream)
        {
            return loadContext.LoadFromStream(stream);
        }

        protected override Assembly Load(AssemblyName assemblyName)
        {
            // First check default context to resolve references
            Assembly asm = null;

            try
            {
                asm = loadContext.LoadFromAssemblyName(assemblyName);
            }
            catch
            {
                // Fallback to default load
                asm = Assembly.Load(assemblyName);
            }

            return asm;
        }

        protected override Assembly LoadFile(string assemblyFile)
        {
            using (var fs = new FileStream(assemblyFile, FileMode.Open, FileAccess.Read))
            {
                return loadContext.LoadFromStream(fs);
            }
        }

        public override void Unload()
        {
            if (loadContext != null)
            {
                contextWeakRef = new WeakReference(loadContext);
                loadContext.Unload();
                loadContext = null;

                // Clear strong references to allow unload
                functions.Clear();

                for (int i = 0; contextWeakRef.IsAlive && i < 10; i++)
                {
                    GC.Collect();
                    GC.WaitForPendingFinalizers();
                    GC.Collect();
                }

                if (contextWeakRef.IsAlive)
                {
                    // TODO: Review this?
                    log.Error("AssemblyLoadContext did not unload.");
                }
                else
                {
                    log.Info("AssemblyLoadContext unloaded successfully.");
                }
            }
        }

        private CollectibleAssemblyLoadContext loadContext;
        private WeakReference contextWeakRef;
    }
}
#endif
