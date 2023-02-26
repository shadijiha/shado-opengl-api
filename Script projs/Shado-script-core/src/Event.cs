using System;
using System.Runtime.InteropServices;
using System.Text;

namespace Shado
{
    public interface IEvent {
    }
    
    [StructLayout(LayoutKind.Sequential)]
    public struct Event : IEvent
    {
        public Int32 categoryFlags;
        public Type type;
        public bool handled;
        
        public bool IsInCategory(Category category) {
            return (categoryFlags & (int)category) == 0;
        }

        public unsafe ref T As<T>() where T : unmanaged, IEvent
        {
            fixed (Event* ptr = &this)
            {
                return ref *(T*)ptr;
            }
        }

        public enum Type : Int32 {
            None = 0,
            WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
            AppTick, AppUpdate, AppRender,
            KeyPressed, KeyReleased, KeyTyped,
            MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
        }

        public enum Category
        {
            None = 0,
            EventCategoryApplication = 1 << 0,
            EventCategoryInput = 1 << 1,
            EventCategoryKeyboard = 1 << 2,
            EventCategoryMouse = 1 << 3,
            EventCategoryMouseButton = 1 << 4
        };

        public override string ToString()
        {
            return Event.FormatString(this);
        }

        internal static string FormatString(IEvent e) {
            StringBuilder builder = new StringBuilder(e.GetType().Name + "[");
            foreach (var field in e.GetType().GetFields()) { 
                if (field.IsPublic)
                    builder.Append(field.Name).Append(": ").Append(field.GetValue(e).ToString()).Append(", ");
            }
            return builder.Append("]").ToString();
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct WindowEvent : IEvent
    {
        public Int32 categoryFlags;
        public Event.Type type;
        public bool handled;

        public UInt32 width;
        public UInt32 height;

        public override string ToString()
        {
            return Event.FormatString(this);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct MouseEvent : IEvent
    {
        public Int32 categoryFlags;
        public Event.Type type;
        public bool handeled;

        private UInt32 width;
        private UInt32 height;

        public float x;
        public float y;
        public int button;

        public override string ToString()
        {
            return Event.FormatString(this);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct KeyEvent : IEvent
    {
        public Int32 categoryFlags;
        public Event.Type type;
        public bool handeled;

        private UInt32 width;
        private UInt32 height;

        private float x;
        private float y;
        private int button;

        public KeyCode keyCode;
        public Int32 repeatCount;

        public override string ToString()
        {
            return Event.FormatString(this);
        }
    }
}
