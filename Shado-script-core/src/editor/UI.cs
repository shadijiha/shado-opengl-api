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

        public static void Image(Texture2D texture, Vector2 size) {
            Image_Native(texture.native, size, new Vector2(0, 1), new Vector2(1, 0));
        }
        
        public static void ImageFromFramebuffer(Framebuffer framebuffer, Vector2 size) {
            Image_Framebuffer_Native(framebuffer.GetColorAttachmentRendererID(0), (uint)size.x, (uint)size.y);
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
        public static extern bool InputInt(string label, ref int value, float speed = 1.0f, int min = 0, int max = 0, string format = "%d");
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool InputFloat(string label, ref float value, float speed = 1.0f, float min = 0.0f, float max = 0.0f, string format = "%.3f");
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint GetId(string label);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetFocus(uint id);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Indent(float x);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Unindent(float x = 0.0f);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void NewLine();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SameLine(float offset = 0.0f, float spacing = -1.0f);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ShowDemoWindow();
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ShowMetricsWindow();
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void BeginGroup();
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void EndGroup();
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool BeginChild(string id, Vector2 size, bool border = false, UIWindowFlags flags = UIWindowFlags.None);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void EndChild();
        
        public static Vector2 GetContentRegionAvail() {
            GetContentRegionAvail_Native(out Vector2 result);
            return result;
        }
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool BeginMenu(string label, bool enabled = true);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void EndMenu();
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool MenuItem(string label, string shortcut = null, bool selected = false, bool enabled = true);

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

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Image_Framebuffer_Native(uint fbColorAttachmentRendererId, uint width, uint height);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetContentRegionAvail_Native(out Vector2 result);

    }
    
    /**
     * enums
     */
    public enum UIWindowFlags {
        None                   = 0,
        NoTitleBar             = 1 << 0,   // Disable title-bar
        NoResize               = 1 << 1,   // Disable user resizing with the lower-right grip
        NoMove                 = 1 << 2,   // Disable user moving the window
        NoScrollbar            = 1 << 3,   // Disable scrollbars (window can still scroll with mouse or programmatically)
        NoScrollWithMouse      = 1 << 4,   // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
        NoCollapse             = 1 << 5,   // Disable user collapsing window by double-clicking on it. Also referred to as "window menu button" within a docking node.
        AlwaysAutoResize       = 1 << 6,   // Resize every window to its content every frame
        NoBackground           = 1 << 7,   // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
        NoSavedSettings        = 1 << 8,   // Never load/save settings in .ini file
        NoMouseInputs          = 1 << 9,   // Disable catching mouse, hovering test with pass through.
        MenuBar                = 1 << 10,  // Has a menu-bar
        HorizontalScrollbar    = 1 << 11,  // Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(ImVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.
        NoFocusOnAppearing     = 1 << 12,  // Disable taking focus when transitioning from hidden to visible state
        NoBringToFrontOnFocus  = 1 << 13,  // Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus)
        AlwaysVerticalScrollbar= 1 << 14,  // Always show vertical scrollbar (even if ContentSize.y < Size.y)
        AlwaysHorizontalScrollbar=1<< 15,  // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
        AlwaysUseWindowPadding = 1 << 16,  // Ensure child windows without border uses style.WindowPadding (ignored by default for non-bordered child windows, because more convenient)
        NoNavInputs            = 1 << 18,  // No gamepad/keyboard navigation within the window
        NoNavFocus             = 1 << 19,  // No focusing toward this window with gamepad/keyboard navigation (e.g. skipped by CTRL+TAB)
        UnsavedDocument        = 1 << 20,  // Display a dot next to the title. When used in a tab/docking context, tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
        NoDocking              = 1 << 21,  // Disable docking of this window

        NoNav                  = NoNavInputs | NoNavFocus,
        NoDecoration           = NoTitleBar | NoResize | NoScrollbar | NoCollapse,
        NoInputs               = NoMouseInputs | NoNavInputs | NoNavFocus,

        // [Internal]
        NavFlattened           = 1 << 23,  // [BETA] Allow gamepad/keyboard navigation to cross over parent border to this child (only use on child that have no scrolling!)
        ChildWindow            = 1 << 24,  // Don't use! For internal use by BeginChild()
        Tooltip                = 1 << 25,  // Don't use! For internal use by BeginTooltip()
        Popup                  = 1 << 26,  // Don't use! For internal use by BeginPopup()
        Modal                  = 1 << 27,  // Don't use! For internal use by BeginPopupModal()
        ChildMenu              = 1 << 28,  // Don't use! For internal use by BeginMenu()
        DockNodeHost           = 1 << 29   // Don't use! For internal use by Begin()/NewFrame()
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
