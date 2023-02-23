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

        public Type GetTargetType() {
            foreach (var attibute in GetAttibutes())
            {
                if (attibute.GetType() == typeof(EditorTargetType)) {
                    return ((EditorTargetType)attibute).targetType;
                }
            }
            return null;
        }
    }

    public class EditorTargetType : System.Attribute { 
        
        internal Type targetType;
        public EditorTargetType(Type type) {
            targetType = type;
        }
    }
}
