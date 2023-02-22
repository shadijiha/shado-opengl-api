using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Shado
{
    public struct Mathf
    {
        internal const float epsilon = 0.0001f;

        public static bool Approx(float a, float b) {
            return a - b <= epsilon;
        }

        public static float Map(float value, float input_start, float input_end, float output_start, float output_end)
        {
            return output_start + ((output_end - output_start) / (input_end - input_start)) * (value - input_start);
        }

        public static float Map01(float value, float input_start, float input_end) {
            return Map(value, input_start, input_end, 0f, 1.0f);
        }

        public static float Clamp(float x, float a, float b) { 
            return Math.Min(Math.Max(x, a), b); ;
        }

        public static float Max(params float[] vals) { 
            float max = float.MinValue;
            foreach (var val in vals)
            {
                if (val > max)
                    max = val;
            }
            return max;
        }

        public static float Min(params float[] vals)
        {
            float min = float.MaxValue;
            foreach (var val in vals)
                if (val < min)
                    min = val;
            return min;
        }

        public static int Floor(float value) {
            return (int)value;
        }

        public static int Round(float value) {
            return (int)Math.Round(value);
        }

        public static int Ceil(float value) { 
            return (int)Math.Ceiling(value);
        }
    }
}
