using System;

namespace CSLoader.Contracts
{

    public interface ILog
    {
        void Info(string message);
        void Debug(string message);
        void Error(string message);
        void Error(string message, Exception ex);
    }

}