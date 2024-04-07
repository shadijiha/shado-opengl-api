using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace Shado
{
    public readonly struct Mathf
    {
        public const double PI = Math.PI;
        internal const float epsilon = 0.0000001f;

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

        private static Random random = new Random();
        public static float Random(float min, float max)
        {
            return (float)((max - min) * random.NextDouble()) + min;
        }
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Abs(float value) {
            return value < 0 ? -value : value;
        }
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Sqrt(float value) {
            return (float)Math.Sqrt(value);
        }
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Sin(float value) {
            return (float)Math.Sin(value);
        }
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Cos(float value) {
            return (float)Math.Cos(value);
        }
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Tan(float value) {
            return (float)Math.Tan(value);
        }
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float DegToRad(float degrees) {
            return degrees * (float)Mathf.PI / 180.0f;
        }
    }
}
