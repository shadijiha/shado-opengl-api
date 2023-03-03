using System;
using System.Runtime.InteropServices;

namespace Shado
{

    [StructLayout(LayoutKind.Sequential)]
    public struct Collision2DInfo
    {
        public Vector2 normal;
        public Points points;
        public Separations separations;

        public override string ToString()
        {
            return $"Collision2DInfo[normal: {normal}, points: {points}, separations: {separations}]";
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Points {
        public Vector2 p1, p2;

        public override string ToString()
        {
            return $"Points[{p1}, {p2}]";
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Separations {
        public float s1, s2;

        public override string ToString()
        {
            return $"Separations[{s1}, {s2}]";
        }
    }
}
