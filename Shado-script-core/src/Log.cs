using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Shado
{
    public class Log
    {
        public enum LogLevel : UInt32
        {
            Trace = 1 << 0,
            Debug = 1 << 1,
            Info = 1 << 2,
            Warn = 1 << 3,
            Error = 1 << 4,
            Critical = 1 << 5
        }

        public static void Info([StringSyntax(StringSyntaxAttribute.CompositeFormat)] string format,
            params object?[] p) {
            unsafe {
                InternalCalls.Log_LogMessage(LogLevel.Info, string.Format(format, p));
            }
        }

        public static void Warn([StringSyntax(StringSyntaxAttribute.CompositeFormat)] string format,
            params object?[] p) {
            unsafe {
                InternalCalls.Log_LogMessage(LogLevel.Warn, string.Format(format, p));
            }
        }

        public static void Error([StringSyntax(StringSyntaxAttribute.CompositeFormat)] string format,
            params object?[] p) {
            unsafe {
                InternalCalls.Log_LogMessage(LogLevel.Error, string.Format(format, p));
            }
        }
    }
}