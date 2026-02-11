using System;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace Shado
{
    [StructLayout(LayoutKind.Explicit)]
    public struct Vector4
    {
        [FieldOffset(0)] public float x;
        [FieldOffset(4)] public float y;
        [FieldOffset(8)] public float z;
        [FieldOffset(12)] public float w;

        public static Vector4 zero  { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => new(0f); }
        public static Vector4 one   { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => new(1f); }

        public Vector4(float scalar)
        {
            x = y = z = w = scalar;
        }

        public Vector4(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public Vector4(Vector3 xyz, float w) : this(xyz.x, xyz.y, xyz.z, w)
        {
        }

        public Vector3 xyz
        {
            get => new(x, y, z);
            set { x = value.x; y = value.y; z = value.z; }
        }
        
        public Vector2 xy
        {
            get => new(x, y);
            set { x = value.x; y = value.y; }
        }

        public Vector4 normalized
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new(x / magnitude, y / magnitude, z / magnitude, w / magnitude);
        }

        public float magnitude
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => (float)Math.Sqrt(magnitudeSquared);
        }

        public float magnitudeSquared
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => x * x + y * y + z * z;
        }
        
        public static Vector4 Lerp(Vector4 a, Vector4 b, float t)
        {
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            return (1.0f - t) * a + t * b;
        }


        #region Operators
        public static Vector4 operator +(Vector4 left, Vector4 right)
        {
            return new Vector4(left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w);
        }

        public static Vector4 operator -(Vector4 left, Vector4 right)
        {
            return new Vector4(left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w);
        }

        public static Vector4 operator *(Vector4 left, Vector4 right)
        {
            return new Vector4(left.x * right.x, left.y * right.y, left.z * right.z, left.w * right.w);
        }

        public static Vector4 operator *(Vector4 left, float scalar)
        {
            return new Vector4(left.x * scalar, left.y * scalar, left.z * scalar, left.w * scalar);
        }

        public static Vector4 operator *(float scalar, Vector4 right)
        {
            return new Vector4(scalar * right.x, scalar * right.y, scalar * right.z, scalar * right.w);
        }

        public static Vector4 operator /(Vector4 left, Vector4 right)
        {
            return new Vector4(left.x / right.x, left.y / right.y, left.z / right.z, left.w / right.w);
        }

        public static Vector4 operator /(Vector4 left, float scalar)
        {
            return new Vector4(left.x / scalar, left.y / scalar, left.z / scalar, left.w / scalar);
        }
        
        public static explicit operator Vector3(Vector4 v)
        {
            return new(v.x, v.y, v.z);
        }
        
        #endregion

        public override string ToString()
        {
            return ToString(null, null);
        }
        
        public string ToString(string? format, IFormatProvider? formatProvider) {
            formatProvider ??= CultureInfo.CurrentCulture;

            string formattedX = x.ToString(format, formatProvider);
            string formattedY = y.ToString(format, formatProvider);
            string formattedZ = z.ToString(format, formatProvider);
            string formattedW = w.ToString(format, formatProvider);

            return $"Vec4({formattedX}, {formattedY}, {formattedZ}, {formattedW})";
        }
    }
}
