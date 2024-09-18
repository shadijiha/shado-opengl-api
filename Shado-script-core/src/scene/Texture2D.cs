using System;
using UI = Shado.Editor.UI;

namespace Shado
{
    public class Texture2D
    {
        public readonly string filepath;
        internal IntPtr native;

        internal Texture2D(string filepath) {
            this.filepath = filepath;
            unsafe {
                fixed (IntPtr* ptr = &this.native) {
                    InternalCalls.Texture2D_Create(filepath, (int**)ptr);
                }
            }
        }

        ~Texture2D() {
            //InternalCalls.Texture2D_Destroy(native);
        }

        public static Texture2D Create(string filepath) {
            if (string.IsNullOrEmpty(filepath))
                return null;
            return new Texture2D(filepath);
        }

        internal void Reset(string filepath) {
            IntPtr newHandle = IntPtr.Zero;
            //InternalCalls.Texture2D_Reset(native, filepath, out newHandle);
            native = newHandle;
        }

        public override string ToString() {
            return $"Texture2D({filepath}, {native})";
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
                texture.Reset(path);
            });
            //UI.SameLine();
            UI.Image(texture, new Vector2(60, 60));
        }

        //void OnEvent(ref Event e) {
        //  Log.Info(ref e);
        //}
    }
}