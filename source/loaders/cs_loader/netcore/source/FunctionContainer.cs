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
        public FunctionContainer(MethodInfo info, object targetInstance, string scriptHandle = null, string overrideName = null )
        {
            //this.FunctionName = $"{info.DeclaringType.FullName}.{info.Name}";
			//change it back later
			this.FunctionName = overrideName ?? info.Name;
			this.ReturnType = info.ReturnType;
            this.Parameters = info.GetParameters();
            this.Assembly = info.Module.Assembly;
            this.Class = info.DeclaringType.FullName;
            this.Method = info;
            this.TargetInstance = targetInstance;
            this.ScriptHandle = scriptHandle;

        }

        public string FunctionName { get; set; }

        public string Class { get; set; }

        public Type ReturnType { get; set; }

        public ParameterInfo[] Parameters { get; set; }

        public Assembly Assembly { get; set; }

        public MethodInfo Method { get; set; }

        public object TargetInstance { get; set; }

        public string ScriptHandle { get; set; }

        public ReflectFunction GetReflectFunction()
        {
            ReflectFunction r = new ReflectFunction();

            r.name = this.FunctionName;
            r.returnType = MetacallDef.Get(this.ReturnType);
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
