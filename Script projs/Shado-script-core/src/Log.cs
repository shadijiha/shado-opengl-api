using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Shado
{
    public class Log
    {
        public static void Info(object format, params object[] p) {
            InternalCalls.Log_Log(string.Format(format.ToString(), p), Type.Info);
        }

        public static void Warn(object format, params object[] p)
        {
            InternalCalls.Log_Log(string.Format(format.ToString(), p), Type.Warn);
        }

        public static void Error(object format, params object[] p)
        {
            InternalCalls.Log_Log(string.Format(format.ToString(), p), Type.Error);
        }

        public static void Critical(object format, params object[] p)
        {
            InternalCalls.Log_Log(string.Format(format.ToString(), p), Type.Critical);
        }

        public static void Trace(object format, params object[] p)
        {
            InternalCalls.Log_Log(string.Format(format.ToString(), p), Type.Trace);
        }

        internal enum Type { 
            Info = 0, Warn, Error, Critical, Trace
        }
    }
}
