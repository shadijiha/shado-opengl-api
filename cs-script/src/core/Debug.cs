using System;
using System.Runtime.CompilerServices;

namespace Shado
{
    public class Debug
    {

        public static void Error(string format, params object[] args) {
            Log(string.Format(format, args), 0);
        }

        public static void Error(object o)
        {
            Log(o.ToString(), 0);
        }

        public static void Warn(string format, params object[] args)
        {
            Log(string.Format(format, args), 1);
        }

        public static void Warn(object o)
        {
            Log(o.ToString(), 1);
        }

        public static void Info(string format, params object[] args)
        {
            Log(string.Format(format, args), 2);
        }

        public static void Info(object o)
        {
            Log(o.ToString(), 2);
        }


        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Log(string message, int type);
    }
}
