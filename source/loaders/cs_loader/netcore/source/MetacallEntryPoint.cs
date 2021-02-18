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
using CSLoader.Providers;

namespace CSLoader
{
    public static class MetacallEntryPoint
    {
        private static LoaderBase loader = null;
        private static ConsoleLog log = new ConsoleLog();

        static MetacallEntryPoint()
        {
            log.Info("CSLoader static initialization");

            #if NETCOREAPP1_0 || NETCOREAPP1_1 || NETCOREAPP1_2
                loader = new Providers.LoaderV1(log);
            #elif NETCOREAPP2_0 || NETCOREAPP2_1 || NETCOREAPP2_2
                loader = new Providers.LoaderV2(log);
            #endif
        }

        public unsafe static bool LoadFromPointer(string[] source)
        {
            return loader.LoadFromSourceFunctions(source);
        }

        public static bool Load(string source)
        {
            return loader.LoadFromSourceFunctions(new string[] { source });
        }

        public static bool Load(string[] files)
        {
            try {
                return loader.LoadFromSourceFunctions(files.Select(x => System.IO.File.ReadAllText(x)).ToArray());
            } catch (FileNotFoundException ex) {
                // TODO: Implement error handling
                log.Info(ex.Message);
                return false;
            }
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
            return (IntPtr)loader.Execute(function, parameters);
        }

        public unsafe static IntPtr ExecuteWithParamsW([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPWStr)] string function,
         [MarshalAs(UnmanagedType.LPArray, SizeConst = 10)]   Parameters[] parameters)
        {
            return (IntPtr)loader.Execute(function, parameters);
        }

        public unsafe static IntPtr ExecuteFunction(string function)
        {
            return (IntPtr)loader.Execute(function);
        }

        public static bool LoadFromAssembly(string assemblyFile)
        {
            return loader.LoadFromAssembly(assemblyFile);
        }

        public static bool LoadFromAssemblyC([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPWStr)] string assemblyFile)
        {
            return LoadFromAssembly(assemblyFile);
        }

        public static bool LoadFromAssemblyW([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string assemblyFile)
        {
            return LoadFromAssembly(assemblyFile);
        }
    }
}