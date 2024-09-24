using System;
using System.Globalization;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Shado
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3 : IEquatable<Vector3>, IComparable<Vector3>, IFormattable
    {
        public float x, y, z;

        public static Vector3 zero {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new(0f);
        }

        public static Vector3 one {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new(1f);
        }

        public Vector3(float scalar) {
            x = scalar;
            y = scalar;
            z = scalar;
        }

        public Vector3(float x, float y, float z) {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public Vector3(Vector2 xy, float z) {
            x = xy.x;
            y = xy.y;
            this.z = z;
        }

        public Vector2 xy {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new Vector2(x, y);
            set {
                this.x = value.x;
                this.y = value.y;
            }
        }

        public Vector3 normalized {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new(x / magnitude, y / magnitude, z / magnitude);
        }

        public float magnitude {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => (float)Math.Sqrt(magnitudeSquared);
        }

        public float magnitudeSquared {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => x * x + y * y + z * z;
        }

        public static float Distance(Vector3 a, Vector3 b) {
            return (float)Math.Sqrt(Math.Pow(b.x - a.x, 2) + Math.Pow(b.y - a.y, 2) + Math.Pow(b.z - a.z, 2));
        }
        
        // Linearly interpolates between two vectors.
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 Lerp(Vector3 a, Vector3 b, float t)
        {
            t = Mathf.Clamp01(t);
            return new Vector3(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t
            );
        }

        // Cross Product of two vectors.
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 Cross(Vector3 lhs, Vector3 rhs)
        {
            return new Vector3(
                lhs.y * rhs.z - lhs.z * rhs.y,
                lhs.z * rhs.x - lhs.x * rhs.z,
                lhs.x * rhs.y - lhs.y * rhs.x);
        }
        
        #region Operators

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator +(Vector3 a, Vector3 b) =>  new(a.x + b.x, a.y + b.y, a.z + b.z);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator -(Vector3 a, Vector3 b) => new(a.x - b.x, a.y - b.y, a.z - b.z);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator *(Vector3 vector, float scalar) => new(vector.x * scalar, vector.y * scalar, vector.z * scalar);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator /(Vector3 vector, float scalar) => new(vector.x / scalar, vector.y / scalar, vector.z / scalar);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator ==(Vector3 a, Vector3 b) =>
            Mathf.Approx(a.magnitudeSquared - b.magnitudeSquared, 0f);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator !=(Vector3 a, Vector3 b) => !(a == b);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator -(Vector3 a) => new(-a.x, -a.y, -a.z);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator >(Vector3 a, Vector3 b)  => a.magnitude > b.magnitude;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator <(Vector3 a, Vector3 b) => a.magnitude < b.magnitude;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator >=(Vector3 a, Vector3 b) => a.magnitude >= b.magnitude;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator <=(Vector3 a, Vector3 b) => a.magnitude <= b.magnitude;

        public static explicit operator Vector2(Vector3 vector) => vector.xy;
        
        #endregion

        public override bool Equals(object? obj) {
            return obj is Vector3 other && Equals(other);
        }

        public override int GetHashCode() {
            unchecked {
                int hashCode = 17;
                hashCode = hashCode * 23 + x.GetHashCode();
                hashCode = hashCode * 23 + y.GetHashCode();
                hashCode = hashCode * 23 + z.GetHashCode();
                return hashCode;
            }
        }

        public bool Equals(Vector3 other) {
            return this == other;
        }

        public int CompareTo(Vector3 other) {
            float magnitudeDiff = magnitude - other.magnitude;

            if (magnitudeDiff > 0)
                return 1;
            if (magnitudeDiff < 0)
                return -1;
            return 0;
        }

        public string ToString(string? format, IFormatProvider? formatProvider) {
            formatProvider ??= CultureInfo.CurrentCulture;

            string formattedX = x.ToString(format, formatProvider);
            string formattedY = y.ToString(format, formatProvider);
            string formattedZ = z.ToString(format, formatProvider);

            return $"Vec3({formattedX}, {formattedY}, {formattedZ})";
        }

        public override string ToString() {
            return ToString(null, null);
        }
    }
}