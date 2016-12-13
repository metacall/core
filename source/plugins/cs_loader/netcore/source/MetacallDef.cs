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
        public enum type_primitive_id : int
        {
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


        private static Dictionary<type_primitive_id, Type> primitiveToType = new Dictionary<type_primitive_id, Type>();

        static MetacallDef()
        {
            foreach (var item in typeToPrimitive.Keys)
            {
                primitiveToType.Add(typeToPrimitive[item], item);
            }
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
