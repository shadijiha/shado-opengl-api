using System;

namespace Shado.Editor
{
    /// <summary>
    /// Use this attribute on a non public class field or property to expose it to the engine editor
    /// </summary>
    ///
    /// <example>
    /// [ShowInEditor] private int count = 0;
    /// </example> 
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class ShowInEditorAttribute : Attribute
    {
        public string DisplayName = "";
        public bool IsReadOnly = false;

        public ShowInEditorAttribute(string displayName = "") {
            DisplayName = displayName;
        }
    }
}