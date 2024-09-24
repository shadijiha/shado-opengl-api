using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Shado
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector2 : IEquatable<Vector2>, IComparable<Vector2>, IFormattable
    {
        public float x, y;

        public static Vector2 zero {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new(0f);
        }

        public static Vector2 one {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new(1f);
        }

        public static Vector2 right {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new(1f, 0f);
        }

        public static Vector2 left {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new(-1f, 0f);
        }

        public static Vector2 up {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new(0f, 1f);
        }

        public static Vector2 down {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new(0f, -1f);
        }

        public Vector2(float scalar) {
            x = scalar;
            y = scalar;
        }

        public Vector2(float x, float y) {
            this.x = x;
            this.y = y;
        }

        #region Operators

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector2 operator +(Vector2 a, Vector2 b) {
            return new Vector2(a.x + b.x, a.y + b.y);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector2 operator -(Vector2 a, Vector2 b) {
            return new Vector2(a.x - b.x, a.y - b.y);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector2 operator *(Vector2 vector, float scalar) {
            return new Vector2(vector.x * scalar, vector.y * scalar);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector2 operator /(Vector2 vector, float scalar) {
            return new Vector2(vector.x / scalar, vector.y / scalar);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector2 operator -(Vector2 a) {
            return a * -1;
        }
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator ==(Vector2 a, Vector2 b) {
            return Mathf.Approx(a.magnitudeSquared - b.magnitudeSquared, 0f);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator !=(Vector2 a, Vector2 b) {
            return !(a == b);
        }
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator >(Vector2 a, Vector2 b) {
            return a.magnitude > b.magnitude;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator <(Vector2 a, Vector2 b) {
            return a.magnitude < b.magnitude;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator >=(Vector2 a, Vector2 b) {
            return a.magnitude >= b.magnitude;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator <=(Vector2 a, Vector2 b) {
            return a.magnitude <= b.magnitude;
        }
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static implicit operator Vector3(Vector2 a) {
            return new Vector3(a.x, a.y, 0f);
        }
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static implicit operator Vector4(Vector2 a) {
            return new Vector4(a.x, a.y, 0f, 0f);
        }
        #endregion

        public Vector2 normalized {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new(x / magnitude, y / magnitude);
        }

        public float magnitude {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => (float)Math.Sqrt(magnitudeSquared);
        }

        public float magnitudeSquared {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => x * x + y * y;
        }

        public static float Distance(Vector2 a, Vector2 b) {
            return (float)Math.Sqrt(Math.Pow(b.x - a.x, 2) + Math.Pow(b.y - a.y, 2));
        }
        
        public bool Equals(Vector2 other) {
            return this == other;
        }

        public int CompareTo(Vector2 other) {
            if (x < other.x) return -1;
            if (x > other.x) return 1;
            if (y < other.y) return -1;
            if (y > other.y) return 1;
            return 0;
        }

        public override string ToString() {
            return ToString(null, null);
        }

        public string ToString(string? format, IFormatProvider? formatProvider) {
            formatProvider ??= System.Globalization.CultureInfo.CurrentCulture;
            return
                $"{nameof(x)}: {x.ToString(format, formatProvider)}, {nameof(y)}: {y.ToString(format, formatProvider)}, {nameof(magnitude)}: {magnitude.ToString(format, formatProvider)}";
        }
    }
}