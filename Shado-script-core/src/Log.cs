using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Shado
{
    public class Log
    {
        public enum LogLevel
        {
            Info,
            //Info = 0, Warn, Error, Critical, Trace
        }

        public static void Info(string format, params object[] p) {
            unsafe {
                InternalCalls.Log_LogMessage(LogLevel.Info, string.Format(format, p));
            }
        }
    }
}