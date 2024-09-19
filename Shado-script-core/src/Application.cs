using System;

namespace Shado
{
    public static class Application
    {
        public static Window window => new Window();

        public static void Close() {
            unsafe {
                InternalCalls.Application_Quit();
            }
        }

        public static Vector2 GetImGuiWindowSize(string windowName) {
            unsafe {
                Vector2 size = Vector2.zero;
                InternalCalls.Application_GetImGuiWindowSize(windowName, &size);
                return size;
            }
        }

        public static bool IsImGuiWindowHovered(string windowName) {
            unsafe {
                return InternalCalls.Application_IsImGuiWindowHovered(windowName);
            }
        }
    }

    public struct Window
    {
        public Vector2 position {
            get {
                unsafe {
                    Vector2 pos;
                    InternalCalls.Window_GetPosition(&pos);
                    return pos;
                }
            }
        }

        public Vector2 size {
            get {
                unsafe {
                    Vector2 size;
                    InternalCalls.Window_GetSize(&size);
                    return size;
                }
            }
            set {
                unsafe {
                    InternalCalls.Window_SetSize(&value);
                }
            }
        }

        public uint width {
            get { return (uint)size.x; }
            set { size = new Vector2(value, height); }
        }

        public uint height {
            get { return (uint)size.y; }
            set { size = new Vector2(width, value); }
        }

        public WindowMode mode {
            get {
                unsafe {
                    return (WindowMode)InternalCalls.Window_GetMode();
                }
            }
            set {
                unsafe {
                    InternalCalls.Window_SetMode((int)value);
                }
            }
        }

        public string title {
            get {
                unsafe {
                    return InternalCalls.Window_GetTitle();
                }
            }
            set {
                unsafe {
                    InternalCalls.Window_SetTitle(value);
                }
            }
        }

        public bool vsync {
            get {
                unsafe {
                    return InternalCalls.Window_GetVsync();
                }
            }
            set {
                unsafe {
                    InternalCalls.Window_SetVsync(value);
                }
            }
        }

        public float opacity {
            set {
                unsafe {
                    InternalCalls.Window_SetOpacity(value);
                }
            }
        }

        public enum WindowMode
        {
            Fullscreen = 0,
            Windowed = 1,
            BorderlessWindowed = 2
        };
    }
}