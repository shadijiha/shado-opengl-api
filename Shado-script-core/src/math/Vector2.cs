using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Shado
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Vector2 : IEquatable<Vector2>, IComparable<Vector2>, IFormattable
	{
		public float x, y;

		public static Vector2 zero { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector2(0f); } }
		public static Vector2 one { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector2(1f); } }
		
		public static Vector2 right { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector2(1f, 0f); } }
		
		public static Vector2 left { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector2(-1f, 0f); } }
		
		public static Vector2 up { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector2(0f, 1f); } }
		
		public static Vector2 down { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector2(0f, -1f); } }

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

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector2 operator +(Vector2 a, Vector2 b)
		{
			return new Vector2(a.x + b.x, a.y + b.y);
		}

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector2 operator -(Vector2 a, Vector2 b) {
			return new Vector2(a.x - b.x, a.y - b.y);
		}

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector2 operator *(Vector2 vector, float scalar)
		{
			return new Vector2(vector.x * scalar, vector.y * scalar);
		}

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector2 operator /(Vector2 vector, float scalar)
		{
			return new Vector2(vector.x / scalar, vector.y / scalar);
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 operator -(Vector2 a) {
			return a * -1;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
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
		
		public static float Distance(Vector2 a, Vector2 b) {
			// TODO: Optimize
			return (a - b).magnitude;	
		}

		private const double TOLERANCE = 0.000000001f;
		
		public bool Equals(Vector2 other) {
			return Mathf.Approx(x, other.x) && Mathf.Approx(y, other.y);
		}
		
		public int CompareTo(Vector2 other) {
			if (x < other.x) return -1;
			if (x > other.x) return 1;
			if (y < other.y) return -1;
			if (y > other.y) return 1;
			return 0;
		}

		public override string ToString() {
			return $"{nameof(x)}: {x}, {nameof(y)}: {y}, {nameof(magnitude)}: {magnitude}";
		}

		public string ToString(string format, IFormatProvider formatProvider) {
			return $"{nameof(x)}: {x.ToString(format, formatProvider)}, {nameof(y)}: {y.ToString(format, formatProvider)}, {nameof(magnitude)}: {magnitude.ToString(format, formatProvider)}";
		}
	}
}
