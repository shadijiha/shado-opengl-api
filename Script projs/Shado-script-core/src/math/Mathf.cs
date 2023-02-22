using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Shado
{
    public struct Mathf
    {
        internal static float epsilon = 0.0001f;

        public static bool Approx(float a, float b) { 
            return a - b <= epsilon;
        }
    }
}
