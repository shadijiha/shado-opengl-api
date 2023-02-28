using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Shado
{
    public struct Collision2DInfo
    {
        public Vector2 normal;
        public Vector2[] points;
        public float[] separations;

        internal Collision2DInfo(Vector2 normal, Vector2[] points, float[] separations) {
            this.normal = normal;
            this.points = points;
            this.separations = separations;
        }
    }
}
