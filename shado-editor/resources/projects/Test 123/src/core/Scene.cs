using System;
using System.Runtime.CompilerServices;

namespace Shado
{
    public struct Scene
    {
        internal static readonly Scene Null = new Scene(IntPtr.Zero);

        private IntPtr native;
        
        internal Scene(IntPtr native) {
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
