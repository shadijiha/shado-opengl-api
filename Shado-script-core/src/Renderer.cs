using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Shado
{
    public static class Renderer
    {

        public static void DrawQuad(Vector3 pos, Vector3 scale, Colour colour) {
            Vector4 temp = (Vector4)colour;
            InternalCalls.Renderer_DrawQuad(ref pos, ref scale, ref temp);
        }

        public static void DrawRotatedQuad(Vector3 pos, Vector3 scale, Vector3 rotation, Colour colour) {
            Vector4 temp = (Vector4)colour;
            InternalCalls.Renderer_DrawRotatedQuad(ref pos, ref scale, ref rotation, ref temp);
        }

        public static void DrawLine(Vector3 p0, Vector3 p1, Colour colour)
        {
            Vector4 temp = (Vector4)colour;
            InternalCalls.Renderer_DrawLine(ref p0, ref p1, ref temp);
        }

        public static void DrawQuad(Vector3 pos, Vector3 scale, Colour colour, Shader shader) {
            Vector4 temp = colour;
            InternalCalls.Renderer_DrawQuadShader(ref pos, ref scale, ref temp, shader.native);
        }
    }
}
