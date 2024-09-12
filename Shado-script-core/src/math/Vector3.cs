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

		public static Vector3 zero	{ [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector3(0f); } }
		public static Vector3 one	{ [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector3(1f); } }

		public Vector3(float scalar)
		{
			x = scalar;
			y = scalar;
			z = scalar;
		}

		public Vector3(float x, float y, float z)
		{
			this.x = x;
			this.y = y;
			this.z = z;
		}

		public Vector3(Vector2 xy, float z)
		{
			x = xy.x;
			y = xy.y;
			this.z = z;
		}

		public Vector2 xy
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get => new Vector2(x, y);
			set
			{
				this.x = value.x;
				this.y = value.y;
			}
		}

        public Vector3 normalized
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get { return new Vector3(x / magnitude, y / magnitude, z / magnitude); }
        }

        public float magnitude
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get { return (float)Math.Sqrt(magnitudeSquared); }
        }

        public float magnitudeSquared
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get { return x * x + y * y + z * z; }
        }
        
        public static float Distance(Vector3 a, Vector3 b)
		{
			return (float)Math.Sqrt(Math.Pow(b.x - a.x, 2) + Math.Pow(b.y - a.y, 2) + Math.Pow(b.z - a.z, 2));
		}
        
        static Random random = new Random();
        public static Vector3 Random(float min, float max)
		{
			return new Vector3(
				(float)random.NextDouble() * (max - min) + min,
				(float)random.NextDouble() * (max - min) + min,
				(float)random.NextDouble() * (max - min) + min);
		}

        #region Operators
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator +(Vector3 a, Vector3 b)
		{
			return new Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
		}

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator -(Vector3 a, Vector3 b)
		{
			return new Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
		}

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator *(Vector3 vector, float scalar)
		{
			return new Vector3(vector.x * scalar, vector.y * scalar, vector.z * scalar);
		}

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator /(Vector3 vector, float scalar)
		{
			return new Vector3(vector.x / scalar, vector.y / scalar, vector.z / scalar);
		}

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator ==(Vector3 a, Vector3 b) {
			return Mathf.Approx(a.magnitudeSquared - b.magnitudeSquared, 0f);
		}

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator !=(Vector3 a, Vector3 b) { 
			return !(a == b);
		}

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator -(Vector3 a) { 
			return new Vector3(-a.x, -a.y, -a.z);
		}

        public static bool operator >(Vector3 a, Vector3 b)
        {
            return a.magnitude > b.magnitude;
        }

        public static bool operator <(Vector3 a, Vector3 b)
        {
            return a.magnitude < b.magnitude;
        }

        public static bool operator >=(Vector3 a, Vector3 b)
        {
            return a.magnitude >= b.magnitude;
        }

        public static bool operator <=(Vector3 a, Vector3 b)
        {
            return a.magnitude <= b.magnitude;
        }
		#endregion
        
		public static explicit operator Vector2(Vector3 vector)
		{
            return vector.xy;
        }

        public override int GetHashCode()
        {
            unchecked
            {
                int hashCode = 17;
                hashCode = hashCode * 23 + x.GetHashCode();
                hashCode = hashCode * 23 + y.GetHashCode();
                hashCode = hashCode * 23 + z.GetHashCode();
                return hashCode;
            }
        }

        public bool Equals(Vector3 other)
        {
			return this == other;
        }

        public int CompareTo(Vector3 other)
        {
            float magnitudeDiff = magnitude - other.magnitude;

            if (magnitudeDiff > 0)
                return 1;
            else if (magnitudeDiff < 0)
                return -1;
            else
                return 0;
        }

        public string ToString(string format, IFormatProvider formatProvider)
        {
            if (formatProvider == null)
                formatProvider = CultureInfo.CurrentCulture;

            string formattedX = x.ToString(format, formatProvider);
            string formattedY = y.ToString(format, formatProvider);
            string formattedZ = z.ToString(format, formatProvider);

            return $"Vec3({formattedX}, {formattedY}, {formattedZ})";
        }

        public override string ToString()
        {
            return $"Vec3({x}, {y}, {z})";
        }

    }
}
