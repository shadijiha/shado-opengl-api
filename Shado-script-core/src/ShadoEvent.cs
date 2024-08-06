using System;
using System.Collections.Generic;
using System.Reflection;
using Shado.Editor;

namespace Shado
{
    public class ShadoEvent 
    {
        internal List<Action<object, object>> funcs = new List<Action<object, object>>();
        
        public static ShadoEvent operator +(ShadoEvent a, Action<object, object> b) {
            if (a is null) a = new ShadoEvent();
            a.funcs.Add(b);
            return a;
        }
        
        public void InvokeAll(object sender, object args) {
            foreach (var func in funcs) {
                func.Invoke(sender, args);
            }
        }
    }
    
    [EditorTargetType(typeof(ShadoEvent))]
    public class ShadoEventEditor : Editor.Editor
    {
        protected override void OnEditorDraw() {
            var handler = (ShadoEvent)target;
            
            UI.NewLine();
            UI.Text(fieldName);
            
            var available = UI.GetContentRegionAvail();
            UI.BeginChild($"##{handler.GetHashCode()}{GetHashCode()}", new Vector2(available.x, 200.0f));
            
            DrawClassesMenu();
            
            foreach (var methodInfo in handler.funcs) {
                UI.Text(methodInfo.Method.ToString());
            }

            UI.EndChild();
            UI.NewLine();
        }

        private void DrawClassesMenu() {
            if (UI.BeginMenu("+##ClassesMenu" + GetHashCode())) {
                var entities = Scene.GetAllEntities();

                foreach (var entity in entities) {
                    
                    if (UI.BeginMenu($"{entity.tag} ({entity.GetType().Name})")) {
                        var methods = entity.GetType().GetMethods(BindingFlags.Instance | BindingFlags.Public);
                        foreach (var method in methods) {
                            if (method.ReturnType == typeof(void) && method.GetParameters().Length == 2) {
                                var methodInfo = method;
                                if (UI.MenuItem(method.Name)) {
                                    var handler = (ShadoEvent)target;
                                    handler.funcs.Add((a, b) => methodInfo.Invoke(entity, new object[] {a, b}));
                                }
                            }
                        }
                        UI.EndMenu();
                    }
                }
                UI.EndMenu();
            }
            
        }
    }
}