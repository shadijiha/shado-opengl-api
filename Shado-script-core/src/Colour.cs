using System;
using System.Runtime.InteropServices;

namespace Shado
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Colour
    {
        public float r, g, b, a;

        // TODO: make aggressive inline
        public static readonly Colour Black = new Colour(0, 0, 0);
        public static readonly Colour White = new Colour(1.0f, 1.0f, 1.0f);
        public static readonly Colour Red = new Colour(1.0f, 0, 0);
        public static readonly Colour Green = new Colour(0, 1.0f, 0);
        public static readonly Colour Blue = new Colour(0, 0, 1.0f);
        public static readonly Colour Yellow = new Colour(1.0f, 1.0f, 0);
        public static readonly Colour Maganta = new Colour(1.0f, 0, 1.0f);
        public static readonly Colour Cyan = new Colour(0, 1.0f, 1.0f);

        public Colour(float r, float g, float b, float a)
        {
            this.r = r;
            this.g = g;
            this.b = b;
            this.a = a;
        }

        public Colour(float r, float g, float b)
            : this(r, g, b, 1.0f) { }

        public Colour(float rgb) : this(rgb, rgb, rgb) { }

        public Colour(float rgb, float a) : this(rgb, rgb, rgb, a) { }

        /**
         * @param hue from 0 to 360
         * @param saturation from 0 to 1
         * @param lightness from 0 to 1
         */
        public static Colour FromHSL(float hue, float saturation, float lightness)
        {
            var C = (1 - Math.Abs(2 * lightness - 1)) * saturation;
            var Hp = hue / 60.0f;
            var X = C * (1 - Math.Abs(Hp % 2 - 1));

            double r1 = 0, g1 = X, b1 = C;

            var m = lightness - (C / 2.0f);

            return new Colour(
                (float)(r1 + m),
                (float)(g1 + m),
                (float)(b1 + m)
            );
        }

        public static Colour Lerp(Colour a, Colour b, float t)
        {
            return new Colour(
                a.r + (b.r - a.r) * t,
                a.g + (b.g - a.g) * t,
                a.b + (b.b - a.b) * t,
                a.a + (b.a - a.a) * t
            );
        }

        private readonly static Random random = new Random();
        public static Colour Random()
        {
            return new Colour((float)random.NextDouble(), (float)random.NextDouble(), (float)random.NextDouble());
        }

        public static explicit operator Vector3(Colour c)
        {
            return new Vector3(c.r, c.g, c.b);
        }

        public static implicit operator Vector4(Colour c)
        {
            return new Vector4(c.r, c.g, c.b, c.a);
        }

        public override string ToString()
        {
            return $"Colour({r}, {g}, {b}, {a})";
        }
    }
}
