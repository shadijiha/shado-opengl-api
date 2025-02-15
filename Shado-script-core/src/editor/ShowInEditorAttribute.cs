using System;

namespace Shado.Editor
{
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