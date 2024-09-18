namespace Shado
{
    public class Input
    {
        public static bool IsKeyDown(KeyCode keycode) {
            unsafe {
                return InternalCalls.Input_IsKeyPressed(keycode);
            }
        }

        public static Vector2 mousePosition {
            get {
                Vector2 pos = default;
                unsafe {
                    InternalCalls.Input_GetMousePosition(&pos);
                }

                return pos;
            }
        }
    }
}