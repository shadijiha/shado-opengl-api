using System;
using System.Runtime.CompilerServices;

namespace Shado
{
    public struct Scene
    {
        private IntPtr native;

        internal Scene(IntPtr native) {
            Console.WriteLine(native);
            this.native = native;
        }

        public Entity GetPrimaryCameraEntity() {
            return GetPrimaryCameraEntity_Native(native);
        }

        internal IntPtr GetNative() { return native; }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Entity GetPrimaryCameraEntity_Native(IntPtr scene);
    }
}
