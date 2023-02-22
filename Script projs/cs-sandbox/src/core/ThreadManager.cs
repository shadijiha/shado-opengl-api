using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace Shado
{
    /**
     * MUST USE this class to create any threads. Otherwise 
     * threads might exit after scene gets destroyed and 
     * cause NULL ptr exceptions
     */
    public static class ThreadManager
    {
        private static List<Thread> threads = new List<Thread>();

        public static Thread Create(ThreadStart start, int maxStackSize = 0) {
            Thread t = new Thread(start, maxStackSize) ;
            threads.Add(t);
            return t;
        }

        public static Thread Create(ParameterizedThreadStart start, int maxStackSize = 0)
        {
            Thread t = new Thread(start, maxStackSize);
            threads.Add(t);
            return t;
        }

        /**
         * Called from C++ to kill all created threads
         */
        private static void KillAll() {
            foreach (Thread t in threads)
            {
                t.Abort();
            }
        }
    }
}
