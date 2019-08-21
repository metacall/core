#if NETCOREAPP1_0 || NETCOREAPP1_1

using System;
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