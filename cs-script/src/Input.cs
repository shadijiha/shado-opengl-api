using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using Shado.math;

namespace Shado
{
    public class Input
    {

        public static bool IsKeyPressed(KeyCode keycode)
        {
            return IsKeyPressed_Native(keycode);
        }

        public static bool IsMouseButtonPressed(MouseCode code) {
            return IsMouseButtonPressed_Native((int)code);
        }

        public static Vector2 GetMousePos()
        {
            Vector2 result;
            GetMousePos_Native(out result);
            return result;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsKeyPressed_Native(KeyCode keycode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsMouseButtonPressed_Native(int keycode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector2 GetMousePos_Native(out Vector2 result);
    }
}
