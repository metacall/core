using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using static CSLoader.MetacallDef;

namespace CSLoader
{
    public class FunctionContainer
    {
        public FunctionContainer(MethodInfo info)
        {
            this.FunctionName = info.Name;
            this.RetunType = info.ReturnType;
            this.Parameters = info.GetParameters();
            this.Assembly = info.Module.Assembly;
            this.Class = info.DeclaringType.FullName;

            this.Method = this.Assembly.GetType(this.Class).GetTypeInfo().GetMethod(this.FunctionName);
        }

        public string FunctionName { get; set; }

        public string Class { get; set; }

        public Type RetunType { get; set; }

        public ParameterInfo[] Parameters { get; set; }

        public Assembly Assembly { get; set; }

        public MethodInfo Method { get; set; }

        public ReflectFunction GetReflectFunction()
        {
            ReflectFunction r = new ReflectFunction();

            r.name = this.FunctionName;
            r.returnType = MetacallDef.Get(this.RetunType);
            r.paramcount = this.Parameters.Length;
            r.pars = new ReflectParam[10];

            for (int i = 0; i < r.paramcount; i++)
            {
                r.pars[i] = ReflectParam.From(this.Parameters[i]);
            }

            return r;
        }
    }
}
