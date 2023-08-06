using System;
using System.Runtime.CompilerServices;
using System.Text;

namespace Shado.Editor
{
    public static class UI
    {
        public enum FileChooserType
        {
            Open = 0, Save, Folder
        };

        public delegate void OnInputFileChooserChanged(string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Begin(string label);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void End();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Text(string text);

        public static void Image(Texture2D texture, Vector2 dimension) {
            Image_Native(texture.native, dimension, new Vector2(0, 1), new Vector2(1, 0));
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Separator();

        public static bool Button(string label)
        {
            return Button_Native(label, Vector2.zero);
        }

        public static void InputTextFileChoose(string label, string text, string[] extensions, Action<string> onChanged, FileChooserType type = FileChooserType.Open) {
            bool changed = InputTextFileChoose_Native(label, text, extensions, out string resultPath, type);
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
            return OpenFileDialog_Native(buffer.ToString(), type);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool InputText(string label, ref string value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint GetId(string label);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetFocus(uint id);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Indent(float x);

        /**
         * Native functions
         */
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Image_Native(IntPtr texture, Vector2 dimension, Vector2 uv0, Vector2 uv1);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Button_Native(string label, Vector2 size);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool InputTextFileChoose_Native(string label, string text, string[] extensions, out string newPath, FileChooserType type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern string OpenFileDialog_Native(string filters, FileChooserType type);
    }

    public static class NodeEditor {

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void BeginNodeEditor();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void EndNodeEditor();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void BeginNode(int id);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void EndNode();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void BeginNodeTitleBar();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void EndNodeTitleBar();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void BeginInputAttribute(int id);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void EndInputAttribute();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void BeginOutputAttribute(int id);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void EndOutputAttribute();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Link(int id, int start_attrib_id, int end_attrib_id);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool IsNodeHovered(ref int id);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool IsLinkHovered(ref int id);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool IsPinHovered(ref int id);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool IsLinkCreated(ref int atrrib_start, ref int atrrib_end);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void IsLinkDestroyed(ref int id);
    }
}
