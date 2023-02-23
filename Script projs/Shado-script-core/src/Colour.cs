using System;

namespace Shado
{
    public struct Colour
    {
        int r, g, b, a;

        // TODO: make aggressive inline
        public static readonly Colour Black = new Colour(0, 0, 0);
        public static readonly Colour White = new Colour(255, 255, 255);
        public static readonly Colour Red = new Colour(255, 0, 0);
        public static readonly Colour Green = new Colour(0, 255, 0);
        public static readonly Colour Blue = new Colour(0, 0, 255);
        public static readonly Colour Yellow = new Colour(255, 255, 0);
        public static readonly Colour Maganta = new Colour(255, 0, 255);
        public static readonly Colour Cyan = new Colour(0, 255, 255);

        public Colour(int r, int g, int b, int a)
        {
            this.r = r;
            this.g = g;
            this.b = b;
            this.a = a;
        }

        public Colour(int r, int g, int b)
            : this(r, g, b, 255) { }

        public Colour(int rgb) : this(rgb, rgb, rgb) { }

        public Colour(int rgb, int a) : this(rgb, rgb, rgb, a) { }

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
                (int)((r1 + m) * 255),
                (int)((g1 + m) * 255),
                (int)((b1 + m) * 255)
            );
        }

        public static explicit operator Vector3(Colour c)
        {
            return new Vector3(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f);
        }

        public static implicit operator Vector4(Colour c)
        {
            return new Vector4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
        }
    }
}
