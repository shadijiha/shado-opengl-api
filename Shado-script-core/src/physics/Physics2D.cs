using System.Runtime.InteropServices;
using Coral.Managed.Interop;

namespace Shado
{
    public static class Physics2D
    {
        public static RaycastHit2D[] Raycast(RaycastData2D raycastData) {
            unsafe {
                return InternalCalls.Physics2D_Raycast(&raycastData);
            }
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct RaycastData2D
    {
        public Vector2 Origin;
        public Vector2 Direction;
        public float MaxDistance;

        private readonly float
            Padding; // NOTE(Peter): Manual padding because C# doesn't fully respect the 8-byte alignment here

        public NativeArray<ReflectionType> RequiredComponents;
    }

    public readonly struct RaycastHit2D
    {
        public readonly ulong EntityID;
        public readonly Vector2 Position;
        public readonly Vector2 Normal;
        public readonly float Distance;

        //public Entity? Entity => Scene.FindEntityByID(EntityID);
    }
}