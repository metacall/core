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

namespace CSLoader.Contracts
{

    public interface ILoader
    {
        ReflectFunction[] Functions();

        string LoadFromSourceFunctions(string[] source);
        void LoadFunctions(Assembly assembly, string scriptHandle);
        string LoadFromAssembly(string assemblyFile);

        unsafe ExecutionResult* Execute(string function, Parameters[] parameters);
        unsafe ExecutionResult* Execute(string function);

        unsafe ExecutionResult* CreateExecutionResult(bool failed, type_primitive_id type);
        unsafe ExecutionResult* CreateExecutionResult(bool failed, type_primitive_id type, object value);
    }
}
