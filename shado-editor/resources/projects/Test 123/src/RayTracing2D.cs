using System;
using System.Collections.Generic;
using System.Reflection;
using Shado;
using Shado.Editor;

namespace Sandbox
{
    #if false
    public class RayTracing2D : Entity
    {
        public float velocity = 1.0f;
        public float angleStep = 1.0f;
        public List<Ray> rays = new List<Ray>();
        
        // TODO: find why these crash
        //public int[] arr = new int[10];
        public ShadoEvent eventHandler = new ShadoEvent();
        
        void OnCreate() {
            eventHandler += Queen;
            eventHandler += (a, b) => { Log.Info("{0} XD", 1); };
            for (float i = 0; i < 360; i += angleStep) {
                var ray = Create<Ray>(() => new Ray(this, 
                    new Vector2(Mathf.Cos(Mathf.DegToRad(i)), Mathf.Sin(Mathf.DegToRad(i))))
                );
                ray.tag = "Ray " + i;
                rays.Add(ray);
            }
        }
        
        void OnUpdate(float dt)
        {
            // TODO Figure out how to do it with mouse input
            if (Input.IsKeyDown(KeyCode.A)) {
                transform.position += new Vector3(-velocity, 0, 0) * dt;
            } else if (Input.IsKeyDown(KeyCode.D)) {
                transform.position += new Vector3(velocity, 0, 0) * dt;
            }
            if (Input.IsKeyDown(KeyCode.W)) {
                transform.position += new Vector3(0, velocity, 0) * dt;
            } else if (Input.IsKeyDown(KeyCode.S)) {
                transform.position += new Vector3(0, -velocity, 0) * dt;
            }
            
            // Check for intersection
            foreach (var ray in rays) {
                float record = float.MaxValue;
                Vector2? recordPoint = null;
                
                foreach (var wall in Boundary.walls) {
                    if (ray.Cast(wall, out Vector2 intersection)) {
                        var d = Vector2.Distance(ray.transform.position.xy, intersection);
                        if (d < record) {
                            record = d;
                            recordPoint = intersection;
                        }
                    }
                }
                
                if (recordPoint.HasValue) {
                    ray.lineRenderer.colour = Colour.Red;
                    ray.lineRenderer.target = recordPoint.Value;
                } else {
                    ray.lineRenderer.colour = Colour.White;
                }
            }
        }
        
        void OnDraw() {
            //Renderer.DrawQuad(new Vector3(0, 0, 0), new Vector3(1, 1, 0), Colour.Blue);         
        }

        public void Queen(object sender, object args) {
            
        }
        
        void OnDestroy() {

        }
    }

    [EditorTargetType(typeof(int[]))]
    public class IntArrayEditor : Editor
    {
        protected override void OnEditorDraw() {
            UI.NewLine();
            UI.Text(fieldName);
            UI.BeginGroup();

            var width = UI.GetContentRegionAvail();
            if (UI.BeginChild("##Scrolling", new Vector2(width.x, 200))) {
                UI.Indent(20.0f);
                var array = (int[])target;
                for (int i = 0; i < Mathf.Min(array.Length, 20); i++) {
                    UI.InputInt($"{i}: ", ref array[i]);
                }
                UI.Unindent();
                UI.NewLine();
            }
            UI.EndChild();
            UI.EndGroup();
        }
    }

    
    public class Boundary : Entity
    {
        public static readonly List<Boundary> walls = new List<Boundary>();
        
        public LineRendererComponent lineRenderer;
        
        void OnCreate() {
            walls.Add(this);
            if (!HasComponent<LineRendererComponent>())
                AddComponent<LineRendererComponent>();
            lineRenderer = GetComponent<LineRendererComponent>();
        }

    }

    public class Ray : Entity
    {
        public Vector2 dir;
        public LineRendererComponent lineRenderer;
        public Entity parent;

        public Ray(Entity parent, Vector2 dir) {
            this.dir = dir;
            this.parent = parent;
        }

        void OnCreate() {
            lineRenderer = AddComponent<LineRendererComponent>();
        }

        void OnUpdate(float dt) {
            this.transform.position = parent.transform.position;
            lineRenderer.target = dir * 10.0f;
        }

        public bool Cast(Boundary wall, out Vector2 intersection) {
                    var x1 = wall.transform.position.x;
                    var y1 = wall.transform.position.y;
                    var x2 = wall.lineRenderer.target.x;
                    var y2 = wall.lineRenderer.target.y;

                    var x3 = this.transform.position.x;
                    var y3 = this.transform.position.y;
                    var x4 = lineRenderer.target.x;
                    var y4 = lineRenderer.target.y;

                    var den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
                    if (den == 0) {
                        intersection = default;
                        return false;
                    }

                    var t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / den;
                    var u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / den;

                    if (t > 0 && t < 1 && u > 0) {
                        intersection = new Vector2(x1 + t * (x2 - x1), y1 + t * (y2 - y1));
                        return true;
                    }
                    
                    intersection = default;
                    return false;
        }
    }
#endif
}