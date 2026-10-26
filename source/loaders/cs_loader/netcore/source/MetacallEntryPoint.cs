using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using System.IO;
using Microsoft.CodeAnalysis.Emit;
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
            log.Info("CSLoader Initialization - Runtime Version: "
                + RuntimeEnvironment.GetSystemVersion()
                + " - Language Version: "
                + LanguageVersionFacts.ToDisplayString(LanguageVersionFacts.MapSpecifiedToEffectiveVersion(LanguageVersion.Latest)));

            #if NETCOREAPP1_0 || NETCOREAPP1_1 || NETCOREAPP1_2
                loader = new Providers.LoaderV1(log);
            #elif NETCOREAPP2_0 || NETCOREAPP2_1 || NETCOREAPP2_2 || NET5_0 || NET7_0 // TODO: Implement or test support for 3
                loader = new Providers.LoaderV2(log);
            #else
            #   error "NET Core platform not supported"
            #endif
        }

        public unsafe static bool LoadFromPointer(string[] source)
        {
            try {
                return loader.LoadFromSourceFunctions(source) != null;
            } catch (Exception ex) {
                // TODO: Implement error handling
                log.Error(ex.Message, ex);
                return false;
            }
        }

        public static bool ExecutionPathC([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPWStr)]string path)
        {
            return loader.ExecutionPath(path);
        }

        public static bool ExecutionPathW([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)]string path)
        {
            return loader.ExecutionPath(path);
        }

        public static bool Load(string source)
        {
            try
            {
                return loader.LoadFromSourceFunctions(new string[] { source }) != null;
            }
            catch (Exception ex)
            {
                // TODO: Implement error handling
                log.Error(ex.Message, ex);
                return false;
            }
        }

        public static bool Load(string[] files)
        {
            try
            {
                return loader.LoadFromFileFunctions(files);
            }
            catch (Exception ex)
            {
                // TODO: Implement error handling
                log.Error(ex.Message, ex);
                return false;
            }
        }

        public static ReflectFunction[] GetFunctionsInternal()
        {
            return loader.Functions();
        }

        private static int WriteMetadata<T>(Func<T[]> getRecords, ref int count, IntPtr destination, int capacity) where T : struct
        {
            // Must match metadata_status in defs.h.
            const int success = 0;
            const int insufficientCapacity = 1;
            const int failure = 2;
            const int invalidArgument = 3;

            count = -1;

            if (capacity < 0 ||
                (destination == IntPtr.Zero && capacity != 0))
            {
                return invalidArgument;
            }

            try
            {
                T[] records = getRecords();
                count = records.Length;

                // A null destination with zero capacity is a count query.
                if (destination == IntPtr.Zero)
                {
                    return success;
                }

                // Validate capacity before writing any records.
                if (capacity < records.Length)
                {
                    return insufficientCapacity;
                }

                if (records.Length == 0)
                {
                    return success;
                }

                int recordSize = Marshal.SizeOf<T>();
                IntPtr current = destination;

                for (int i = 0; i < records.Length; ++i)
                {
                    Marshal.StructureToPtr(records[i], current, false);

                    if (i + 1 < records.Length)
                    {
                        current = IntPtr.Add(current, recordSize);
                    }
                }

                return success;
            }
            catch (Exception ex)
            {
                count = -1;

                // Logging must not allow an exception to cross the bridge.
                try
                {
                    log.Error("Failed to export metadata", ex);
                }
                catch (Exception)
                {
                }

                return failure;
            }
        }

        public static int GetFunctionsV2(ref int count, IntPtr destination, int capacity)
        {
            return WriteMetadata<ReflectFunction>(
                loader.Functions, ref count, destination, capacity);
        }

        public static int GetClassesV2(ref int count, IntPtr destination, int capacity)
        {
            return WriteMetadata<ReflectClass>(
                loader.Classes, ref count, destination, capacity);
        }

        public static int GetConstructorsV2(ref int count, IntPtr destination, int capacity)
        {
            return WriteMetadata<ReflectConstructor>(
                loader.Constructors, ref count, destination, capacity);
        }

        public static int GetMethodsV2(ref int count, IntPtr destination, int capacity)
        {
            return WriteMetadata<ReflectMethod>(
                loader.Methods, ref count, destination, capacity);
        }

        public static int GetAttributesV2(ref int count, IntPtr destination, int capacity)
        {
            return WriteMetadata<ReflectAttribute>(
                loader.Attributes, ref count, destination, capacity);
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

        public static IntPtr CreateObjectC([MarshalAs(UnmanagedType.LPStr)] string className, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)] Parameters[] parameters, long size)
        {
            return loader.CreateObject(
                className,
                parameters,
                size);
        }

        public unsafe static IntPtr InvokeObjectC(IntPtr handle, [MarshalAs(UnmanagedType.LPStr)] string method, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] Parameters[] parameters, long size)
        {
            return (IntPtr)loader.InvokeObject(
                handle,
                method,
                parameters,
                size);
        }

        public unsafe static IntPtr InvokeStaticC([MarshalAs(UnmanagedType.LPStr)] string className, [MarshalAs(UnmanagedType.LPStr)] string method, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] Parameters[] parameters, long size)
        {
            return (IntPtr)loader.InvokeStatic(
                className,
                method,
                parameters,
                size);
        }

        public unsafe static IntPtr GetObjectAttributeC(IntPtr handle, [MarshalAs(UnmanagedType.LPStr)] string attribute)
        {
            return (IntPtr)loader.GetObjectAttribute(
                handle,
                attribute);
        }

        public static int SetObjectAttributeC(IntPtr handle, [MarshalAs(UnmanagedType.LPStr)] string attribute, [MarshalAs(UnmanagedType.LPArray, SizeConst = 1)] Parameters[] parameters)
        {
            return loader.SetObjectAttribute(
                handle,
                attribute,
                parameters) ? 0 : 1;
        }

        public unsafe static IntPtr GetStaticAttributeC([MarshalAs(UnmanagedType.LPStr)] string className, [MarshalAs(UnmanagedType.LPStr)] string attribute)
        {
            return (IntPtr)loader.GetStaticAttribute(
                className,
                attribute);
        }

        public static int SetStaticAttributeC([MarshalAs(UnmanagedType.LPStr)] string className, [MarshalAs(UnmanagedType.LPStr)] string attribute, [MarshalAs(UnmanagedType.LPArray, SizeConst = 1)] Parameters[] parameters)
        {
            return loader.SetStaticAttribute(
                className,
                attribute,
                parameters) ? 0 : 1;
        }

        public static void DestroyObjectC(IntPtr handle)
        {
            loader.DestroyObject(handle);
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
            return loader.LoadFromAssembly(assemblyFile) != null;
        }

        public static bool LoadFromAssemblyC([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPWStr)] string assemblyFile)
        {
            return LoadFromAssembly(assemblyFile);
        }

        public static bool LoadFromAssemblyW([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string assemblyFile)
        {
            return LoadFromAssembly(assemblyFile);
        }

        public static void Destroy()
        {
            loader.Unload();
            loader = null;
        }
    }
}
