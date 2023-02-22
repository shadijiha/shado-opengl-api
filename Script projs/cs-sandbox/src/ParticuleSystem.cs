using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using Shado.math;
using System;

namespace Shado
{
    [StructLayout(LayoutKind.Sequential)]
    public struct ParticuleProps {
       public Vector3 position;
       public Vector3 velocity;
       public Vector4 colorBegin;
       public Vector4 colorEnd;
       public float sizeBegin;
       public float sizeEnd;
       public float sizeVariation;
       public float lifeTime;
    }

    public class ParticuleSystem
    {
        private IntPtr native;

        public ParticuleSystem() {
            native = Constrcut_Native();
        }

        ~ParticuleSystem() {
            Destroy_Native(native);
        }

        public void Emit(ParticuleProps props) {
            Emit_Native(native, ref props);
        }

        public void OnUpdate(float dt) {
            OnUpdate_Native(native, dt);
        }
        public uint Count()
        {
            return Count_Native(native);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr Constrcut_Native();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Destroy_Native(IntPtr ptr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void OnUpdate_Native(IntPtr ptr, float dt);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Emit_Native(IntPtr ptr, ref ParticuleProps dt);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern uint Count_Native(IntPtr ptr);
    }
}
