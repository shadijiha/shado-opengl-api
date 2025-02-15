using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace Shado
{
    public readonly struct Mathf
    {
        public const double PI = Math.PI;
        internal const float epsilon = 0.0000001f;
        
        private static Random random = new Random();

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool Approx(float a, float b) {
            return a - b <= epsilon;
        }

        public static T Map<T>(T value, T inputStart, T inputEnd, T outputStart, T outputEnd) where T : INumber<T> {
            return outputStart + ((outputEnd - outputStart) / (inputEnd - inputStart)) * (value - inputStart);
        }

        public static T Map01<T>(T value, T inputStart, T inputEnd) where T : INumber<T> {
            return Map<T>(value, inputStart, inputEnd, T.Zero, T.One);
        }

        public static T Clamp<T>(T x, T a, T b) where T : INumber<T> { 
            return T.Clamp(x, a, b);
        }
        
        public static T Clamp01<T>(T x) where T : INumber<T> {
            return Clamp(x, T.Zero, T.One);
        }

        public static T Max<T>(params T[] vals) where T : INumber<T> {
            T max = vals[0];
            for(int i = 1; i < vals.Length; i++)
            {
                if (vals[i] > max)
                    max = vals[i];
            }
            return max;
        }

        public static T Min<T>(params T[] vals) where T : INumber<T>
        {
            T min = vals[0];
            for(int i = 1; i < vals.Length; i++)
            {
                if (vals[i] < min)
                    min = vals[i];
            }
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

        
        public static float Random(float min = 0.0f, float max = 1.0f)
        {
            return (float)((max - min) * random.NextDouble()) + min;
        }
 
        public static Vector3 RandomVec3(float min, float max) => new(
                (float)random.NextDouble() * (max - min) + min,
                (float)random.NextDouble() * (max - min) + min,
                (float)random.NextDouble() * (max - min) + min);
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static T Abs<T>(T value) where T : INumber<T> {
            return value < T.Zero ? -value : value;
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
