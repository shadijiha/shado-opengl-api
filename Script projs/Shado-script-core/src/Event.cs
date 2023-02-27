using System;
using System.Runtime.InteropServices;
using System.Text;

namespace Shado
{
    public interface IEvent {
    }
    
    /**
     * This struct and All similar Event structs should be passed BY REFERENCE ONLY
     * DO NOT COPY IT OR ELSE IT WILL GIVE UNDEFINED BEHAVIOUR
     * 
     * The reason being that these 4 structs are TYPE PUNNED. And when C# tries to copy the struct,
     * since it doesn't know which is the correct type, C# will copy only the first 3 fields and ignore the rest
     * 
     * TODO: User unions instead of this trash to avoid that problem
     */
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
            return $"{type.ToString("G")}[Handeled: {handled}]";
        }

        internal static string ToString(ref Event e) {
             if (e.type >= Type.MouseButtonPressed && e.type <= Type.MouseScrolled)
                 return e.As<MouseEvent>().ToString();
             if (e.type >= Type.KeyPressed && e.type <= Type.KeyTyped)
                 return e.As<KeyEvent>().ToString();
             if (e.type >= Type.WindowClose && e.type <= Type.WindowMoved)
                 return e.As<WindowEvent>().ToString();
            return e.ToString();
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
            return $"{type.ToString("G")}[Width: {width}, Height: {height}]";
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
            return $"{type.ToString("G")}[Handeled: {handeled}, X: {x}, Y: {y}, Button: {button}]";
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
            return $"{type.ToString("G")}[Handeled: {handeled}, KeyCode: {keyCode}, RepeatCount: {repeatCount}]";
        }
    }
}
