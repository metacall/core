using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace CSLoader
{
    public class MetacallDef
    {
        public enum type_primitive_id : short
        {
            /* TODO: Update this properly for new types */
            TYPE_BOOL = 0,
            TYPE_CHAR = 1,
            TYPE_SHORT = 2,
            TYPE_INT = 3,
            TYPE_LONG = 4,
            TYPE_FLOAT = 5,
            TYPE_DOUBLE = 6,
            TYPE_STRING = 7,
            TYPE_PTR = 8
        };

        [System.Runtime.InteropServices.StructLayout(System.Runtime.InteropServices.LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public struct ReflectFunction
        {
            public type_primitive_id returnType;
            public int paramcount;
            [System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.ByValTStr, SizeConst = 100)]
            public string name;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 10)]
            public ReflectParam[] pars;
        }

        [System.Runtime.InteropServices.StructLayout(System.Runtime.InteropServices.LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public struct ReflectParam
        {
            public type_primitive_id type;
            [System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.ByValTStr, SizeConst = 100)]
            public string name;

            public static ReflectParam From(ParameterInfo info)
            {
                ReflectParam m = new ReflectParam();
                m.name = info.Name;
                m.type = MetacallDef.Get(info.ParameterType);
                return m;
            }
        }


        [System.Runtime.InteropServices.StructLayout(System.Runtime.InteropServices.LayoutKind.Sequential)]
        public struct ExecutionResult
        {
            public type_primitive_id type;
            public bool failed;
            public IntPtr ptr;
        }

        //[System.Runtime.InteropServices.StructLayout(System.Runtime.InteropServices.LayoutKind.Sequential)]
        public struct Parameters
        {
            public type_primitive_id type;
            public IntPtr ptr;
        }

        private static Dictionary<Type, type_primitive_id> typeToPrimitive = new Dictionary<Type, type_primitive_id>()
        {
            [typeof(byte)] = type_primitive_id.TYPE_CHAR,
            [typeof(short)] = type_primitive_id.TYPE_SHORT,
            [typeof(int)] = type_primitive_id.TYPE_INT,
            [typeof(bool)] = type_primitive_id.TYPE_BOOL,
            [typeof(long)] = type_primitive_id.TYPE_LONG,
            [typeof(float)] = type_primitive_id.TYPE_FLOAT,
            [typeof(double)] = type_primitive_id.TYPE_DOUBLE,
            [typeof(object)] = type_primitive_id.TYPE_PTR,
            [typeof(string)] = type_primitive_id.TYPE_STRING
        };

        private static Dictionary<type_primitive_id, Func<IntPtr, object>> primitiveToValue = new Dictionary<type_primitive_id, Func<IntPtr, object>>()
        {
            [type_primitive_id.TYPE_CHAR] = (ptr) => { return Marshal.PtrToStructure<byte>(ptr); },
            [type_primitive_id.TYPE_SHORT] = (ptr) => { return Marshal.PtrToStructure<short>(ptr); },
            [type_primitive_id.TYPE_INT] = (ptr) => { return Marshal.PtrToStructure<int>(ptr); },
            [type_primitive_id.TYPE_BOOL] = (ptr) => { return Marshal.PtrToStructure<bool>(ptr); },
            [type_primitive_id.TYPE_LONG] = (ptr) => { return Marshal.PtrToStructure<long>(ptr); },
            [type_primitive_id.TYPE_FLOAT] = (ptr) => { return Marshal.PtrToStructure<float>(ptr); },
            [type_primitive_id.TYPE_DOUBLE] = (ptr) => { return Marshal.PtrToStructure<double>(ptr); },
            [type_primitive_id.TYPE_PTR] = (ptr) => { return ptr; },
            [type_primitive_id.TYPE_STRING] = (ptr) => { return Marshal.PtrToStringAnsi(ptr); }
        };

        private static Dictionary<type_primitive_id, Func<object, IntPtr>> primitiveToIntPtr = new Dictionary<type_primitive_id, Func<object, IntPtr>>()
        {
            [type_primitive_id.TYPE_CHAR] = (obj) => { IntPtr ptr = Marshal.AllocHGlobal(sizeof(byte)); Marshal.StructureToPtr((byte)obj, ptr, false); return ptr; },
            [type_primitive_id.TYPE_SHORT] = (obj) => { IntPtr ptr = Marshal.AllocHGlobal(sizeof(short)); Marshal.StructureToPtr((short)obj, ptr, false); return ptr; },
            [type_primitive_id.TYPE_INT] = (obj) => { IntPtr ptr = Marshal.AllocHGlobal(sizeof(int)); Marshal.StructureToPtr((int)obj, ptr, false); return ptr; },
            [type_primitive_id.TYPE_BOOL] = (obj) => { IntPtr ptr = Marshal.AllocHGlobal(sizeof(bool)); Marshal.StructureToPtr((bool)obj, ptr, false); return ptr; },
            [type_primitive_id.TYPE_LONG] = (obj) => { IntPtr ptr = Marshal.AllocHGlobal(sizeof(long)); Marshal.StructureToPtr((long)obj, ptr, false); return ptr; },
            [type_primitive_id.TYPE_FLOAT] = (obj) => { IntPtr ptr = Marshal.AllocHGlobal(sizeof(float)); Marshal.StructureToPtr((float)obj, ptr, false); return ptr; },
            [type_primitive_id.TYPE_DOUBLE] = (obj) => { IntPtr ptr = Marshal.AllocHGlobal(sizeof(double)); Marshal.StructureToPtr((double)obj, ptr, false); return ptr; },
            [type_primitive_id.TYPE_PTR] = (obj) => { return (IntPtr)obj; },
            [type_primitive_id.TYPE_STRING] = (obj) => { return Marshal.StringToHGlobalAnsi((string)obj); },
        };

        private static Dictionary<type_primitive_id, Type> primitiveToType = new Dictionary<type_primitive_id, Type>();

        static MetacallDef()
        {
            foreach (var item in typeToPrimitive.Keys)
            {
                primitiveToType.Add(typeToPrimitive[item], item);
            }
        }

        public static object GetValue(type_primitive_id type, IntPtr ptr)
        {
            return primitiveToValue[type](ptr);
        }

        public static IntPtr GetIntPtr(type_primitive_id type, object obj)
        {
            return primitiveToIntPtr[type](obj);
        }

        public static type_primitive_id Get(Type type)
        {
            if (typeToPrimitive.ContainsKey(type))
            {
                return typeToPrimitive[type];
            }
            else
            {
                return type_primitive_id.TYPE_PTR;
            }
        }

        public static Type Get(type_primitive_id primitive)
        {
            if (primitiveToType.ContainsKey(primitive))
            {
                return primitiveToType[primitive];
            }
            else
            {
                return typeof(object);
            }
        }
    }
}
