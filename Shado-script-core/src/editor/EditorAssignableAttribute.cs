using System;

namespace Shado.Editor
{
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = true)]
    internal class EditorAssignableAttribute : Attribute
    { }
}