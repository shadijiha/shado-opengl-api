using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Shado.math
{
    [StructLayout(LayoutKind.Explicit)]
    public struct Vector4
    {
        [FieldOffset(0)] public float X;
        [FieldOffset(4)] public float Y;
        [FieldOffset(8)] public float Z;
        [FieldOffset(12)] public float W;

        public Vector4(float scalar)
        {
            X = Y = Z = W =scalar;
        }

        public Vector4(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        public Vector3 XYZ
        {
            get { return new Vector3(X, Y, Z); }
            set { X = value.X; Y = value.Y; Z = value.Z; }
        }

        public static Vector4 operator+(Vector4 left, Vector4 right)
        {
            return new Vector4(left.X + right.X, left.Y + right.Y, left.Z + right.Z, left.W + right.W);
        }

        public static Vector4 operator-(Vector4 left, Vector4 right)
        {
            return new Vector4(left.X - right.X, left.Y - right.Y, left.Z - right.Z, left.W - right.W);
        }

        public static Vector4 operator*(Vector4 left, Vector4 right)
        {
            return new Vector4(left.X * right.X, left.Y * right.Y, left.Z * right.Z, left.W * right.W);
        }

        public static Vector4 operator *(Vector4 left, float scalar)
        {
            return new Vector4(left.X * scalar, left.Y * scalar, left.Z * scalar, left.W * scalar);
        }

        public static Vector4 operator *(float scalar, Vector4 right)
        {
            return new Vector4(scalar * right.X, scalar * right.Y, scalar * right.Z, scalar * right.W);
        }

        public static Vector4 operator/(Vector4 left, Vector4 right)
        {
            return new Vector4(left.X / right.X, left.Y / right.Y, left.Z / right.Z, left.W / right.W);
        }

        public static Vector4 operator /(Vector4 left, float scalar)
        {
            return new Vector4(left.X / scalar, left.Y / scalar, left.Z / scalar, left.W / scalar);
        }

        public static Vector4 Lerp(Vector4 a, Vector4 b, float t)
        {
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            return (1.0f - t) * a + t * b;
        }

        public static implicit operator Vector3(Vector4 v) {
            return new Vector3(v.X, v.Y, v.Z);
        }

        public override string ToString()
        {
            return $"{{{X}, {Y}, {Z}, {W}}}";
        }

    }
}
