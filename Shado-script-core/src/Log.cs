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
            InternalCalls.Log_Log(Process(format, p), Type.Info);
        }

        public static void Info(ref Event e) { 
            Log.Info(Event.ToString(ref e));
        }

        public static void Warn(object format, params object[] p)
        {
            InternalCalls.Log_Log(Process(format, p), Type.Warn);
        }

        public static void Error(object format, params object[] p)
        {
            InternalCalls.Log_Log(Process(format, p), Type.Error);
        }

        public static void Critical(object format, params object[] p)
        {
            InternalCalls.Log_Log(Process(format, p), Type.Critical);
        }

        public static void Trace(object format, params object[] p)
        {
            InternalCalls.Log_Log(Process(format, p), Type.Trace);
        }

        private static string Process(object format, params object[] p) {
            if (format is Event e) {
                Warn("Attempting to Log object of type Event with the normal Log function. Use Log.<func>(ref Event)");
            }

            for (int i = 0; i < p.Length; i++) {
                if (p[i] is Event ev)
                    Warn("Attempting to Log object of type Event with the normal Log function. Use Log.<func>(ref Event)");
            }

            return string.Format(format.ToString(), p);
        }

        internal enum Type { 
            Info = 0, Warn, Error, Critical, Trace
        }
    }
}
