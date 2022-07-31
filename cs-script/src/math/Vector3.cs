using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Shado.math
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3
    {
        public float x;
        public float y;
        public float z;

        public static readonly Vector3 Zero = new Vector3(0, 0, 0);
        public static readonly Vector3 One = new Vector3(1, 1, 1);

        public Vector3(float scalar)
        {
            x = y = z = scalar;
        }

        public Vector3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public Vector3(Vector2 vector)
        {
            x = vector.x;
            y = vector.y;
            z = 0.0f;
        }

        public Vector3(Vector4 vector)
        {
            x = vector.x;
            y = vector.y;
            z = vector.z;
        }

        public Vector2 xy
        {
            get { return new Vector2(x, y); }
            set { x = value.x; y = value.y; }
        }
        public Vector2 xz
        {
            get { return new Vector2(x, z); }
            set { x = value.x; z = value.y; }
        }
        public Vector2 yz
        {
            get { return new Vector2(y, z); }
            set { y = value.x; z = value.y; }
        }

        public void Normalize() { 
            var mag = (float)Math.Sqrt(x * x + y * y + z * z);
            x /= mag;
            y /= mag;
            z /= mag;
        }

        public static Vector3 operator +(Vector3 v1, Vector3 v2) { 
            return new Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
        }

        public static Vector3 operator -(Vector3 v1, Vector3 v2)
        {
            return new Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
        }
        public static Vector3 operator *(Vector3 v1, Vector3 v2)
        {
            return new Vector3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
        }

        public static Vector3 operator *(Vector3 v1, float scalar)
        {
            return new Vector3(v1.x * scalar, v1.y * scalar, v1.z * scalar);
        }

        public static Vector3 operator /(Vector3 v1, Vector3 v2)
        {
            return new Vector3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
        }

        public static Vector3 operator /(Vector3 v1, float scalar)
        {
            return new Vector3(v1.x / scalar, v1.y / scalar, v1.z / scalar);
        }


        public static Vector3 operator -(Vector3 vector)
        {
            return new Vector3(-vector.x, -vector.y, -vector.z);
        }

        public static implicit operator Vector2(Vector3 v)
        {
            return new Vector2(v.x, v.y);
        }

        public static explicit operator Vector4(Vector3 v)
        {
            return new Vector4(v.x, v.y, v.z, 1.0f);
        }

        public override string ToString()
        {
            return $"{{{x}, {y}, {z}}}";
        }
    }
}
