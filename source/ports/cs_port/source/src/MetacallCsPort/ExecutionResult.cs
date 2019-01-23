using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace MetacallCsPort
{
    public class ExecutionResult : IDisposable
    {
        private IntPtr ptr;

        private ExecutionResult(IntPtr ptr)
        {
            this.ptr = ptr;
        }

        public static ExecutionResult Make(IntPtr ptr)
        {
            return new ExecutionResult(ptr);
        }

        public void Dispose()
        {
            if (this.ptr != IntPtr.Zero)
            {
                MetacallAPI.metacall_value_destroy(this.ptr);
            }
        }

        public bool HasValue
        {
            get
            {
                return (this.ptr != IntPtr.Zero);
            }
        }

        public int AsInt()
        {
            return Marshal.PtrToStructure<int>(this.ptr);
        }

        public byte AsByte()
        {
            return Marshal.PtrToStructure<byte>(this.ptr);
        }

        public long AsLong()
        {
            return Marshal.PtrToStructure<long>(this.ptr);
        }

        public string AsString()
        {
            return Marshal.PtrToStringAnsi(this.ptr);
        }

        public T As<T>() where T : struct
        {
            return Marshal.PtrToStructure<T>(this.ptr);
        }
    }
}
