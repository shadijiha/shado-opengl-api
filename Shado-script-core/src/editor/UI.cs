using System;
using System.Runtime.CompilerServices;
using System.Text;

namespace Shado.Editor
{
    public static class UI
    {
        public enum FileChooserType
        {
            Open = 0,
            Save,
            Folder
        };

        public delegate void OnInputFileChooserChanged(string path);

        public static void Image(Texture2D texture, Vector2 size) {
            //Image_Native(texture.native, size, new Vector2(0, 1), new Vector2(1, 0));
        }

        // public static void ImageFromFramebuffer(Framebuffer framebuffer, Vector2 size) {
        //     Image_Framebuffer_Native(framebuffer.GetColorAttachmentRendererID(0), (uint)size.x, (uint)size.y);
        // }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Separator();

        public static bool Button(string label) {
            //return Button_Native(label, Vector2.zero);
            return false;
        }

        public static void InputTextFileChoose(string label, string text, string[] extensions, Action<string> onChanged,
            FileChooserType type = FileChooserType.Open) {
            string resultPath = text;
            bool changed = false; //InputTextFileChoose_Native(label, text, extensions, out string resultPath, type);
            if (changed && resultPath != null) {
                onChanged(resultPath);
            }
        }

        public static string OpenFileDialog(string[] extensions, FileChooserType type) {
            StringBuilder buffer = new StringBuilder();

            // TODO: Fix this
            int count = 0;
            foreach (var ext in extensions) {
                buffer.Append("*" + ext);

                if (count != extensions.Length - 1)
                    buffer.Append(";");
                count++;
            }

            return "";
            //return OpenFileDialog_Native(buffer.ToString(), type);
        }
    }
}