using System;

namespace Shado
{
    public static class Application
    {
        public static Window window => new Window();
        
        public static void Close() {
            InternalCalls.Application_Close();
        }

        public static Vector2 GetImGuiWindowSize(string windowName) {
            Vector2 size;
            InternalCalls.Application_GetImGuiWindowSize(windowName, out size);
            return size;
        }
        
        public static bool IsImGuiWindowHovered(string windowName) {
            return InternalCalls.Application_IsImGuiWindowHovered(windowName);
        }
    }

    public struct Window
    {
        public Vector2 position {
            get {
                Vector2 pos;
                InternalCalls.Window_GetPosition(out pos);
                return pos;
            }
        }
        
        public Vector2 size {
            get {
                Vector2 size;
                InternalCalls.Window_GetSize(out size);
                return size;
            }
            set {
                InternalCalls.Window_SetSize(value);
            }
        }

        public uint width {
            get { return (uint)size.x; }
            set {
                size = new Vector2(value, height);
            }
        }

        public uint height {
            get { return (uint)size.y; }
            set {
                size = new Vector2(width, value);
            }
        }
        
        public WindowMode mode {
            get {
                return (WindowMode)InternalCalls.Window_GetMode();
            }
            set {
                InternalCalls.Window_SetMode((int)value);
            }
        }
        
        public string title {
            get {
                return InternalCalls.Window_GetTitle();
            }
            set {
                InternalCalls.Window_SetTitle(value);
            }
        }
        
        public bool vsync {
            get {
                return InternalCalls.Window_GetVsync();
            }
            set {
                InternalCalls.Window_SetVsync(value);
            }
        }

        public float opacity { 
            set { InternalCalls.Window_SetOpacity(value); }
        }
        
        public enum WindowMode {
            Fullscreen = 0, Windowed = 1, BorderlessWindowed = 2
        };
    }
}