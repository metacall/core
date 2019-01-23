using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace MetacallCsPort
{
    public interface IFunction
    {
        ExecutionResult Invoke();

        ExecutionResult Invoke(params object[] args);

        void InvokeNonResult();

        void InvokeNonResult(params object[] args);
    }
}
