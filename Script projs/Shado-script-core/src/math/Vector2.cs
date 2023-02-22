using System;
using System.Runtime.CompilerServices;

namespace Shado
{
	public struct Vector2
	{
		public float x, y;

		public static Vector3 zero { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector3(0f); } }
		public static Vector3 one { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector3(1f); } }

		public Vector2(float scalar)
		{
			x = scalar;
			y = scalar;
		}

		public Vector2(float x, float y)
		{
			this.x = x;
			this.y = y;
		}

		public static Vector2 operator +(Vector2 a, Vector2 b)
		{
			return new Vector2(a.x + b.x, a.y + b.y);
		}

		public static Vector2 operator -(Vector2 a, Vector2 b) {
			return new Vector2(a.x - b.x, a.y - b.y);
		}

		public static Vector2 operator *(Vector2 vector, float scalar)
		{
			return new Vector2(vector.x * scalar, vector.x * scalar);
		}

		public static Vector2 operator /(Vector2 vector, float scalar)
		{
			return new Vector2(vector.x / scalar, vector.x / scalar);
		}

		public static implicit operator Vector3(Vector2 a) {
			return new Vector3(a.x, a.y, 0f);
		}

		public Vector2 normalized {
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get { return new Vector2(x / magnitude, y / magnitude); } 
		}

		public float magnitude
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get { return (float)Math.Sqrt(magnitudeSquared); }
		}

		public float magnitudeSquared
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get { return x * x + y * y; }
		}
	}
}
