#if NETCOREAPP1_0 || NETCOREAPP1_1 || NETCOREAPP1_2

using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.IO;
using System.Runtime.Loader;
using CSLoader.Contracts;

namespace CSLoader.Providers
{
    public class LoaderV1 : LoaderBase
    {
        private Assembly Context_Resolving(AssemblyLoadContext context, AssemblyName name)
        {
            Assembly asm = null;

            this.log.Info("CSLoader resolving assembly " + name.Name);

            foreach (var path in paths)
            {
                this.log.Info("CSLoader resolving path " + path);

                try
                {
                    asm = context.LoadFromAssemblyPath(Path.Combine(path, args.Name + ".dll"));

                    if (asm != null)
                    {
                        return asm;
                    }
                }
                catch (Exception ex)
                {
                    this.log.Error(ex.Message, ex);
                }
            }

            return asm;
        }

        public LoaderV1(ILog log) : base(log)
        {
            log.Info("CSLoaderV1 static initialization");

            AssemblyLoadContext.Default.Resolving += this.Context_Resolving;
        }

        protected override IEnumerable<string> AdditionalLibs()
        {
            return Array.Empty<string>();
        }

        protected override Assembly MakeAssembly(MemoryStream stream)
        {
            return AssemblyLoadContext.Default.LoadFromStream(stream);
        }

        protected override Assembly Load(AssemblyName assemblyName)
        {
            return AssemblyLoadContext.Default.LoadFromAssemblyName(assemblyName);
        }

        protected override Assembly LoadFile(string assemblyFile)
        {
            return AssemblyLoadContext.Default.LoadFromAssemblyPath(assemblyFile);
        }
    }
}
#endif
