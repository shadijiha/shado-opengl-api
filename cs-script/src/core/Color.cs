using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Shado.math;

namespace Shado
{
    public struct Color
    {
        int r, g, b, a;

        public static readonly Color Black = new Color(0, 0, 0);
        public static readonly Color White = new Color(1, 1, 1);
        public static readonly Color Red = new Color(1, 0, 0);
        public static readonly Color Green = new Color(0, 1, 0);
        public static readonly Color Blue = new Color(0, 0, 1);
        public static readonly Color Yellow = new Color(1, 1, 0);
        public static readonly Color Maganta = new Color(1, 0, 1);
        public static readonly Color Cyan = new Color(0, 1, 1);

        public Color(int r, int g, int b, int a) { 
            this.r = r;
            this.g = g;              
            this.b = b;
            this.a = a;
        }

        public Color(int r, int g, int b)
            : this(r, g, b, 255) {}

        public Color(int rgb) : this(rgb, rgb, rgb) { }

        public Color(int rgb, int a) : this(rgb, rgb, rgb, a) { }

        /**
         * @param hue from 0 to 360
         * @param saturation from 0 to 1
         * @param lightness from 0 to 1
         */
        public static Color fromHSL(float hue, float saturation, float lightness) {

            var C = (1 - Math.Abs(2 * lightness - 1)) * saturation;
            var Hp = hue / 60.0f;
            var X = C * (1 - Math.Abs(Hp % 2 - 1));

            double r1 = 0, g1 = X, b1 = C;

            var m = lightness - (C / 2.0f);

            return new Color(
                (int)((r1 + m) * 255),
                (int)((g1 + m) * 255),
                (int)((b1 + m) * 255)
            );
        }

        public static implicit operator Vector4(Color c) { 
            return new Vector4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
        }

    }
}
