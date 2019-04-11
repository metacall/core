using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace MetacallCsPort
{
    internal static class MetacallAPI
    {
        const string libraryName = "metacall";

        public enum metacall_value_id : int
        {
            METACALL_BOOL = 0,
            METACALL_CHAR = 1,
            METACALL_SHORT = 2,
            METACALL_INT = 3,
            METACALL_LONG = 4,
            METACALL_FLOAT = 5,
            METACALL_DOUBLE = 6,
            METACALL_STRING = 7,
            METACALL_PTR = 8,

            METACALL_SIZE,
            METACALL_INVALID
        };

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern int metacall_initialize();

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern IntPtr metacallv([MarshalAs(UnmanagedType.LPStr)] string function,
             IntPtr[] args);

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern IntPtr metacall_function([MarshalAs(UnmanagedType.LPStr)] string function);

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern IntPtr metacallfv(IntPtr function, IntPtr[] args);

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern int metacall_load_from_file([MarshalAs(UnmanagedType.LPStr)] string tag, [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPStr)] string[] path, long size);

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern int metacall_load_from_file([MarshalAs(UnmanagedType.LPStr)] string tag, IntPtr[] path, long size);

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern int metacall_load_from_package([MarshalAs(UnmanagedType.LPStr)] string tag, [MarshalAs(UnmanagedType.LPStr)] string path);

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern int metacall_load_from_memory([MarshalAs(UnmanagedType.LPStr)] string tag,
            [MarshalAs(UnmanagedType.LPStr)] string buffer,
            long size);

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern int metacall_value_destroy(IntPtr ptr);

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern IntPtr metacall_value_create_int(int value);

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern IntPtr metacall_value_create_short(short value);

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern IntPtr metacall_value_create_char(byte value);

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern IntPtr metacall_value_create_bool(int value);

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern IntPtr metacall_value_create_long(long value);

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern IntPtr metacall_value_create_float(float value);

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern IntPtr metacall_value_create_double(double value);

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern IntPtr metacall_value_create_string([MarshalAs(UnmanagedType.LPStr)]string value, long size);

        [DllImport(libraryName, CharSet = CharSet.Ansi)]
        public static extern IntPtr metacall_value_create_ptr(IntPtr ptr);
    }
}
