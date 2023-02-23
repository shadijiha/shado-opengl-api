using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Shado
{
    public class Texture2D
    {
        public readonly string filepath;
        internal IntPtr native;

        internal Texture2D(IntPtr native, string filepath) { 
            this.filepath = filepath;
            this.native = native;
        }

        ~Texture2D() {
            InternalCalls.Texture2D_Destroy(native);
        }

        public static Texture2D Create(string filepath) {
            if (string.IsNullOrEmpty(filepath))
                return null;
            return InternalCalls.Texture2D_Create(filepath);
        }

        public override string ToString()
        {
            return $"Texture2D({filepath}, {native})";
        }
    }
}
