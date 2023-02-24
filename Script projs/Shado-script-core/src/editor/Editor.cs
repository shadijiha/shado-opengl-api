using System;

namespace Shado.Editor
{
    public abstract class Editor
    {
        protected object target;
        protected string fieldName;

        protected abstract void OnEditorDraw();

        internal object[] GetAttibutes() {
            return GetType().GetCustomAttributes(true);
        }

        public string GetTargetType() {
            foreach (var attibute in GetAttibutes())
            {
                if (attibute.GetType() == typeof(EditorTargetType)) {
                    return ((EditorTargetType)attibute).targetType.FullName;
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
