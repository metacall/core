using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using static CSLoader.MetacallDef;

namespace CSLoader
{
    public sealed class ClassContainer
    {
        public Type Type { get; }

        public string ScriptHandle { get; }

        public string ClassName => this.Type.Name;

        public ClassContainer(Type type, string scriptHandle)
        {
            this.Type = type;
            this.ScriptHandle = scriptHandle;
        }

        private static ReflectParam[] GetParameters(ParameterInfo[] parameters)
        {
            ReflectParam[] result = new ReflectParam[10];

            for (int i = 0; i < parameters.Length && i < result.Length; ++i)
            {
                result[i] = new ReflectParam
                {
                    type = MetacallDef.Get(parameters[i].ParameterType),
                    name = parameters[i].Name ?? string.Empty
                };
            }

            return result;
        }

        public ReflectClass GetReflectClass()
        {
            return new ReflectClass
            {
                name = this.ClassName
            };
        }

        public ReflectConstructor[] GetReflectConstructors()
        {
            return this.Type
                .GetConstructors(
                    BindingFlags.Public |
                    BindingFlags.Instance |
                    BindingFlags.DeclaredOnly)
                .Select(constructor => new ReflectConstructor
                {
                    className = this.ClassName,
                    paramcount = constructor.GetParameters().Length,
                    pars = GetParameters(constructor.GetParameters())
                })
                .ToArray();
        }

        public ReflectMethod[] GetReflectMethods()
        {
            return this.Type
                .GetMethods(
                    BindingFlags.Public |
                    BindingFlags.Instance |
                    BindingFlags.Static |
                    BindingFlags.DeclaredOnly)
                .Select(method => new ReflectMethod
                {
                    className = this.ClassName,
                    returnType = MetacallDef.Get(method.ReturnType),
                    paramcount = method.GetParameters().Length,
                    name = method.Name,
                    isStatic = method.IsStatic ? 1 : 0,
                    pars = GetParameters(method.GetParameters())
                })
                .ToArray();
        }

        public ReflectAttribute[] GetReflectAttributes()
        {
            return this.Type
                .GetFields(
                    BindingFlags.Public |
                    BindingFlags.Instance |
                    BindingFlags.Static |
                    BindingFlags.DeclaredOnly)
                .Select(field => new ReflectAttribute
                {
                    className = this.ClassName,
                    type = MetacallDef.Get(field.FieldType),
                    name = field.Name,
                    isStatic = field.IsStatic ? 1 : 0
                })
                .ToArray();
        }
    }
}
