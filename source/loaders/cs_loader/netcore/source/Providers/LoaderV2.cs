#if NETCOREAPP2_0 || NETCOREAPP2_1 || NETCOREAPP2_2

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
            log.Info("CSLoader static initialization");

            AppDomain.CurrentDomain.AssemblyResolve += new ResolveEventHandler(AssemblyResolveEventHandler);
            AppDomain.CurrentDomain.TypeResolve += new ResolveEventHandler(AssemblyResolveEventHandler);
        }

        private Assembly AssemblyResolveEventHandler(object sender, ResolveEventArgs args)
        {
            Assembly asm = null;

            foreach (var path in paths)
            {
                try
                {
                    var fullPath = path + "\\" + args.Name + ".dll";

                    this.log.Info(fullPath);

                    asm = Assembly.LoadFile(fullPath);

                    if (asm != null)
                    {
                        return asm;
                    }
                    else
                    {
                        log.Error("Invalid Assembly.LoadFile: " + fullPath);
                    }
                }
                catch (Exception ex)
                {
                    log.Error(string.Format("Exception when loading the Assembly {0}: {1}", args.Name, ex.Message), ex);
                }
            }

            return asm;
        }


        protected override IEnumerable<string> AdditionalLibs()
        {
            return System.IO.Directory.GetFiles(AppDomain.CurrentDomain.BaseDirectory, "*.dll");
        }

        protected override Assembly MakeAssembly(MemoryStream stream)
        {
            return Assembly.Load(stream.ToArray());
        }

        protected override Assembly Load(AssemblyName assemblyName)
        {
            return Assembly.Load(assemblyName);
        }

        protected override Assembly LoadFile(string assemblyFile)
        {
            return Assembly.LoadFile(assemblyFile);
        }
    }
}
#endif
