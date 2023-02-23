using System;

namespace Shado.Editor
{
    public abstract class Editor
    {
        protected object target;

        protected abstract void OnEditorDraw();

        internal object[] GetAttibutes() {
            return GetType().GetCustomAttributes(true);
        }
    }

    public class EditorTargetType : System.Attribute { 
        
        internal Type targetType;
        public EditorTargetType(Type type) {
            targetType = type;
        }
    }
}
