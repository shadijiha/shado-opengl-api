namespace Shado
{
	public class Input
	{
		public static bool IsKeyDown(KeyCode keycode)
		{
			return InternalCalls.Input_IsKeyDown(keycode);
		}

		public static Vector2 mousePosition {
			get {
				Vector2 pos = default;
				InternalCalls.Input_GetMousePos(ref pos);
				return pos;
			}
		}
	}
}
