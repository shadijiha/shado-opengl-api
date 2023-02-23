using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Shado
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Vector3
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

		public static Vector3 operator +(Vector3 a, Vector3 b)
		{
			return new Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
		}

		public static Vector3 operator -(Vector3 a, Vector3 b)
		{
			return new Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
		}

		public static Vector3 operator *(Vector3 vector, float scalar)
		{
			return new Vector3(vector.x * scalar, vector.y * scalar, vector.z * scalar);
		}

		public static Vector3 operator /(Vector3 vector, float scalar)
		{
			return new Vector3(vector.x / scalar, vector.y / scalar, vector.z / scalar);
		}

		public static bool operator ==(Vector3 a, Vector3 b) {
			return Mathf.Approx(a.magnitudeSquared - b.magnitudeSquared, 0f);
		}

		public static bool operator !=(Vector3 a, Vector3 b) { 
			return !(a == b);
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

        public override string ToString()
        {
            return $"Vec3({x}, {y}, {z})";
        }
    }
}
