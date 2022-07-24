using System;
using System.Runtime.CompilerServices;

namespace Shado
{
    public struct Scene
    {
        private IntPtr native;

        internal Scene(IntPtr native) { 
            this.native = native;
        }

        public Entity GetPrimaryCameraEntity() {
            return GetPrimaryCameraEntity(native);
        }

        internal IntPtr GetNative() { return native; }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Entity GetPrimaryCameraEntity(IntPtr scene);
    }
}
