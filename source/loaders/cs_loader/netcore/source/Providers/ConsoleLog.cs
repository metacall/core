using System;
using CSLoader.Contracts;

namespace CSLoader.Providers{
    public class ConsoleLog : ILog
    {
        public void Debug(string message)
        {
            Console.WriteLine(message);
        }

        public void Error(string message)
        {
            Console.Error.WriteLine(message);
        }

        public void Error(string message, Exception ex)
        {
            Console.Error.WriteLine(message);
            Console.Error.WriteLine(ex.StackTrace.ToString());
        }

        public void Info(string message)
        {
            Console.WriteLine(message);
        }
    }
}