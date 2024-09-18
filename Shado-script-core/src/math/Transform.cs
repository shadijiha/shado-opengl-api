using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace Shado
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Transform
    {
        public Vector3 Position;
        public Vector3 Rotation;
        public Vector3 Scale;

        //public Vector3 Up { get => new Quaternion(Rotation) * Vector3.Up; }
        //public Vector3 Right { get => new Quaternion(Rotation) * Vector3.Right; }
        //public Vector3 Forward { get => new Quaternion(Rotation) * Vector3.Forward; }

        public Transform(Vector3 position, Vector3 rotation, Vector3 scale) {
            Position = position;
            Rotation = rotation;
            Scale = scale;
        }

        public Transform Inverse {
            get {
                Transform result = default;
                unsafe {
                    fixed (Transform* thisPtr = &this) {
                        //InternalCalls.TransformInverse_Native(thisPtr, &result);
                    }
                }

                return result;
            }
        }

        public static Transform operator *(Transform a, Transform b) {
            Transform result = default;
            unsafe {
                //InternalCalls.TransformMultiply_Native(&a, &b, &result);
            }

            return result;
        }

        public override bool Equals(object? obj) => obj is Transform other && Equals(other);

        public bool Equals(Transform right) => Position.x == right.Position.x && Position.y == right.Position.y &&
                                               Position.z == right.Position.z && Rotation.x == right.Rotation.x &&
                                               Rotation.y == right.Rotation.y && Rotation.z == right.Rotation.z &&
                                               Scale.x == right.Scale.x && Scale.y == right.Scale.y &&
                                               Scale.z == right.Scale.z;

        public override int GetHashCode() => (Position.x, Position.y, Position.z, Rotation.x, Rotation.y, Rotation.z,
            Scale.x, Scale.y, Scale.z).GetHashCode();

        public static bool operator ==(Transform left, Transform right) => left.Equals(right);
        public static bool operator !=(Transform left, Transform right) => !(left == right);
    }
}