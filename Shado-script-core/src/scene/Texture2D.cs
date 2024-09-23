using System;
using Shado.Editor;
using UI = Shado.Editor.UI;

namespace Shado
{
    [EditorAssignable]
    public class Texture2D
    {
        public readonly string filepath;
        internal ulong handle = 0;

        public Texture2D(string pathRelativeToProject) {
            filepath = pathRelativeToProject;
            unsafe {
                handle = InternalCalls.Texture2D_Create(filepath);
            }
        }

        internal Texture2D(ulong handle) {
            this.handle = handle;
        }

        ~Texture2D() {
            //InternalCalls.Texture2D_Destroy(native);
        }

        public override string ToString() {
            return $"Texture2D({filepath}, {handle})";
        }

        public static implicit operator bool(Texture2D? texture) {
            return texture is not null && texture.handle != 0;
        }
    }

    [Editor.EditorTargetType(typeof(Texture2D))]
    public class Texture2DEditor : Editor.Editor
    {
        static string[] textureExtension = {
            ".jpg", ".png"
        };

        protected override void OnEditorDraw() {
            if (target is null) {
                return;
            }

            Texture2D texture = (Texture2D)target;
            UI.InputTextFileChoose(fieldName, texture.filepath, textureExtension, path => {
                Log.Info(path);
                //texture.Reset(path);
            });
            //UI.SameLine();
            UI.Image(texture, new Vector2(60, 60));
        }

        //void OnEvent(ref Event e) {
        //  Log.Info(ref e);
        //}
    }
}