using System;
using System.Collections;
using System.Collections.Generic;
using System.Threading;
using Shado;
using Shado.Editor;

namespace Sandbox
{ 
    public class Test : Entity
    {
        public float maxDelta;
        public float moveRate;
        public float tilingFactor = 1.0f;
        private TransformComponent transform;
        private float dir = 1;

        public Texture2D texture = Texture2D.Create("Assets/riven2.jpg");
        public Colour colourTest;
        public Vector3 debug_size = Vector3.one;
        public float angle = 0.0f;
        public Shader shader = Shader.Create("Assets/empty.glsl");
        public float totalDt = 0.0f;

        private Test other;
        //private ParticuleSystem particuleSystem;

        void OnCreate() {
            transform = GetComponent<TransformComponent>();
            GetComponent<SpriteRendererComponent>().texture = texture;
            //var t = this.Create<Test>();
            //t.AddComponent<SpriteRendererComponent>().colour = Colour.Blue;
            //GetComponent<BoxCollider2DComponent>().restitution = 0.6f;

            const float max = 0.0f;
            for (float y = -5.0f; y < max; y += 0.5f)
            {
                for (float x = -5.0f; x < max; x += 0.5f)
                {
                    // TODO: Change the Entity.Create to following signature
                    // Create<T>(param object[] ctorArgs)
                    // The reason for that is because right now it is not possible to invode 
                    // internal methods such as AddComponent in the .ctor
                    GridCell gridCell = Create<GridCell>(() => new GridCell(this, x ,y));
                }
            }

            //particuleSystem = Create<ParticuleSystem>();
            //particuleSystem.OnCreate();
        }

        void OnUpdate(float dt)
        {
            //particuleSystem.OnUpdate(dt);   
            if (this.tag == "Square (2)") {
                float x = 10;
            }
            totalDt += dt;
            GetComponent<SpriteRendererComponent>().colour = colourTest;
            GetComponent<SpriteRendererComponent>().texture = texture;
            Vector3 pos = transform.position;
            pos.x += moveRate * 2 * dt * dir;
            //transform.position = pos;
            
            if (pos.x > maxDelta || pos.x < -maxDelta) {
                dir = -dir;
            }

            if (Input.IsKeyDown(KeyCode.K) && other is null) {
                //other = Create<Test>();
            }
        }

        void OnDraw() {
            //Renderer.DrawQuad(new Vector3(-1, 1, 0), debug_size, Colour.Blue, shader);
            //Renderer.DrawRotatedQuad(new Vector3(2, 1, 0), Vector3.one, new Vector3(angle, angle, angle), Colour.Green);
            //Renderer.DrawLine(new Vector3(-1, 1, 0), new Vector3(2, 1, 0), Colour.Red);
            //angle += 0.01f;
        }

        void OnCollision2DEnter(Collision2DInfo info, Entity other) { 
            Log.Info("Collision enter {0} with {1}", info, other.tag);
            //Destroy(this);
        }

        /*
        void OnCollision2DLeave(Collision2DInfo info, Entity other) {
            Log.Info("Collision leave {0} with {1}", info, other.tag);
        }*/
    }

    public class TestCamera : Entity {

        public float moveDelta = 5.0f;
        private CameraComponent camera;
        private Entity camera2;

        void OnCreate() { 
            if (HasComponent<CameraComponent>())
                camera = GetComponent<CameraComponent>();
            camera2 = FindEntityByName("camera2");
        }

        void OnUpdate(float dt) {
            
            Vector3 pos = translation;
            if (Input.IsKeyDown(KeyCode.W)) {
                pos.y += moveDelta * dt;
            }
            if (Input.IsKeyDown(KeyCode.S))
            {
                pos.y -= moveDelta * dt;
            }
            if (Input.IsKeyDown(KeyCode.A))
            {
                pos.x -= moveDelta * dt;
            }
            if (Input.IsKeyDown(KeyCode.D))
            {
                pos.x += moveDelta * dt;
            }
            if (Input.IsKeyDown(KeyCode.Space))
            {
                camera.primary = false;
                camera2.GetComponent<CameraComponent>().primary = true;
            }
            else {
                camera.primary = true;
                camera2.GetComponent<CameraComponent>().primary = false;
            }
            
            translation = pos;
        }
    }
    
    class GridCell : Entity {
        static readonly Texture2D[] texture = {
            Texture2D.Create(@"Assets\riven.png"),
            Texture2D.Create(@"Assets\riven2.jpg"),
            Texture2D.Create(@"Assets\riven3.png")
        };
        static Random random = new Random();
        static uint collisions = 0;

        internal float x, y;
        internal Entity parent;
        float angle = 0.01f;

        public GridCell(Entity parent, float x, float y) { 
            this.x = x;
            this.y = y;
            this.parent = parent;
        }

        internal void OnCreate() {
            var script = GetComponent<ScriptComponent>();
            Log.Info("Class: {0}, ID: {1}", script.ClassName, this.ID);
            transform.position = parent.transform.position + new Vector3(x, y, 0);
            transform.scale = new Vector3(0.45f, 0.45f, 0);
            tag = $"Grid cell {x}, {y}";

            var sprite = AddComponent<SpriteRendererComponent>();
            //sprite.colour = new Vector4((x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f);
            sprite.colour = Colour.Random();
            sprite.texture = texture[random.Next(texture.Length)];
 
            var rb = AddComponent<RigidBody2DComponent>();
            AddComponent<BoxCollider2DComponent>();
            rb.type = RigidBody2DComponent.BodyType.Dynamic;
        }

        void OnUpdate(float dt)
        {
            
            //GetComponent<SpriteRendererComponent>().colour = Colour.FromHSL(angle % 360, 100, 100);
            angle += dt;
        }

        // This is not working because when creating an Entiy the script is not assigned
        void OnCollision2DEnter(Collision2DInfo info, Entity other)
        {
            collisions++;
            if (collisions % 1000 == 0)
            {
                Log.Info("Collisions {0}", collisions);
            }
            //if (other.tag.ToLower() == "ground")
            //   Destroy(this);
        }

        void OnCollision2DLeave(Collision2DInfo info, Entity other)
        {
            collisions++;
            if (collisions % 1000 == 0)
            {
                Log.Info("Collisions LEAVE {0}", collisions);
            }
        }
    }
}
