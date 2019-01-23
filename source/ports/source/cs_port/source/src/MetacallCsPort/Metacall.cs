using System;
using System.Collections.Generic;
using System.Dynamic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace MetacallCsPort
{
    public class Metacall : DynamicObject
    {
        protected class Function : IFunction
        {
            public IntPtr Pointer { get; private set; }

            private Metacall metacall;

            public Function(Metacall metacall, IntPtr pointer)
            {
                this.Pointer = pointer;
                this.metacall = metacall;
            }

            public ExecutionResult Invoke()
            {
                return metacall.Invoke(this);
            }

            public ExecutionResult Invoke(params object[] args)
            {
                return metacall.Invoke(this, args);
            }

            public void InvokeNonResult()
            {
                this.Invoke().Dispose();
            }

            public void InvokeNonResult(params object[] args)
            {
               this.Invoke(args).Dispose();
            }
        }

        static Metacall()
        {
            MetacallAPI.metacall_initialize();
        }

        public Metacall()
        {
        }

        public bool LoadScriptFromFile(string tag, params string[] files)
        {
            var ff = files.Select(x => Marshal.StringToHGlobalAnsi(x)).ToArray();
            return MetacallAPI.metacall_load_from_file(tag, ff, (long)files.Length) == 0;
        }

        public override bool TryInvokeMember(InvokeMemberBinder binder, object[] args, out object result)
        {
            result = this.Invoke(binder.Name, args);

            return true;
        }

        public ExecutionResult Invoke(string function, params object[] args)
        {
            List<IntPtr> metacallArgs = new List<IntPtr>(args.Length);

            foreach (var arg in args)
            {
                metacallArgs.Add(primitiveToIntPtr[arg.GetType()](arg));
            }

            IntPtr ptr = MetacallAPI.metacallv(function, metacallArgs.ToArray());

            return ExecutionResult.Make(ptr);
        }

        public bool HasFunction(string name)
        {
            return MetacallAPI.metacall_function(name) != IntPtr.Zero;
        }

        public IFunction GetFuntion(string name)
        {
            IntPtr pointer = IntPtr.Zero;
            IFunction result = null;
            pointer = MetacallAPI.metacall_function(name);

            if (pointer != IntPtr.Zero)
            {
                result = new Function(this, pointer);
            }

            return result;
        }

        protected ExecutionResult Invoke(Function function, params object[] args)
        {
            List<IntPtr> metacallArgs = new List<IntPtr>(args.Length);

            foreach (var arg in args)
            {
                metacallArgs.Add(primitiveToIntPtr[arg.GetType()](arg));
            }

            IntPtr ptr = MetacallAPI.metacallfv(function.Pointer, metacallArgs.ToArray());

            return ExecutionResult.Make(ptr);
        }

        private static Dictionary<Type, Func<object, IntPtr>> primitiveToIntPtr = new Dictionary<Type, Func<object, IntPtr>>()
        {
            [typeof(byte)] = (obj) => { return MetacallAPI.metacall_value_create_char((byte)obj); },
            [typeof(short)] = (obj) => { return MetacallAPI.metacall_value_create_short((short)obj); },
            [typeof(int)] = (obj) => { return MetacallAPI.metacall_value_create_int((int)obj); },
            [typeof(bool)] = (obj) => { return MetacallAPI.metacall_value_create_bool((bool)obj ? 1 : 0); },
            [typeof(long)] = (obj) => { return MetacallAPI.metacall_value_create_long((long)obj); },
            [typeof(float)] = (obj) => { return MetacallAPI.metacall_value_create_float((float)obj); },
            [typeof(double)] = (obj) => { return MetacallAPI.metacall_value_create_double((double)obj); },
            [typeof(object)] = (obj) => { return MetacallAPI.metacall_value_create_ptr((IntPtr)obj); },
            [typeof(string)] = (obj) => { return MetacallAPI.metacall_value_create_string((string)obj, (long)((string)obj).Length); },
        };
    }
}
