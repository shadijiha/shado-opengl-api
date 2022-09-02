using System;
using Shado.math;
using System.Runtime.CompilerServices;

namespace Shado
{
    /**
     * Used mostly for debug
     */
    public static class Renderer
    {
        public static void DrawQuad(Vector3 position, Vector3 scale, Vector3 rotation) {
            DrawQuad(position, scale, rotation, new Vector4(1.0f));
        }

        public static void DrawQuad(Vector3 position, Vector3 scale, Vector3 rotation, Vector4 color) {
            DrawQuad_Native(position, scale, rotation, color);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DrawQuad_Native(Vector3 position, Vector3 scale, Vector3 rotation, Vector4 color);

    }
}
