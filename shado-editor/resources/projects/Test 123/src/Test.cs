using System;
using System.Collections.Generic;
using System.Threading;
using Shado;

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

        void OnCreate()
        {
            transform = GetComponent<TransformComponent>();
            GetComponent<SpriteRendererComponent>().texture = texture;
            //GetComponent<BoxCollider2DComponent>().restitution = 0.6f;
        }

        void OnUpdate(float dt)
        {
            totalDt += dt;
            GetComponent<SpriteRendererComponent>().colour = colourTest;
            //GetComponent<SpriteRendererComponent>().texture = texture;
            Vector3 pos = transform.position;
            pos.x += moveRate * 2 * dt * dir;
            transform.position = pos;

            if (pos.x > maxDelta || pos.x < -maxDelta) {
                dir = -dir;
            }
        }

        void OnDraw() {
            //Renderer.DrawQuad(new Vector3(-1, 1, 0), debug_size, Colour.Blue, shader);
            //Renderer.DrawRotatedQuad(new Vector3(2, 1, 0), Vector3.one, new Vector3(angle, angle, angle), Colour.Green);
            //Renderer.DrawLine(new Vector3(-1, 1, 0), new Vector3(2, 1, 0), Colour.Red);
            //angle += 0.01f;
        }
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

    //public class Test : Entity
    //{
    //    private SpriteRendererComponent sprite; 
    //    private Entity camera;
    //    private List<Entity> entities = new List<Entity>();
    //    private bool flag = false;

    //    protected override void OnCreate() {
    //        camera = Scene.GetPrimaryCameraEntity();

    //        Debug.Warn("Transform {0}", Transform.Position);

    //        AddComponent<SpriteRendererComponent>();
    //        AddComponent<RigidBody2DComponent>().Type = RigidBody2DComponent.BodyType.DYNAMIC;
    //        var bcparent = AddComponent<BoxCollider2DComponent>();
    //        bcparent.Restitution = 0.7f;
    //        bcparent.RestitutionThreshold = 0.6f;

    //        sprite = GetComponent<SpriteRendererComponent>();
    //        sprite.Color = new Vector4(0.3f, 1.0f, 0.7f, 1.0f);

    //        Texture2D texture = new Texture2D(@"assets\riven.png");
    //        for (int i = 1; i < 4; i++) { 
    //            Entity entity = Entity.Create();
    //            entity.Transform.Position = Transform.Position + new Vector3(i * 2, 0.0f, 0.0f);
    //            entity.Transform.Rotation = Transform.Rotation;
    //            entity.Tag = "hehexd " + i;

    //            var sprite = entity.AddComponent<CircleRendererComponent>();
    //            sprite.Texture = texture;


    //            var rb = entity.AddComponent<RigidBody2DComponent>();
    //            rb.Type = RigidBody2DComponent.BodyType.DYNAMIC;

    //            var bc = entity.AddComponent<CircleCollider2DComponent>();
    //            bc.Restitution = 0.7f;

    //            entities.Add(entity);
    //        }

    //        //DestroyAfter(3);
    //    }

    //    protected override void OnUpdate(float dt)
    //    {
    //        if (!flag && Input.IsKeyPressed(KeyCode.Space)) {
    //            Debug.Info("hehexd");
    //            entities[0].Destroy();
    //            entities.RemoveAt(0);
    //            flag = true;
    //        }

    //        if (camera != null)
    //        {
    //            if (Input.IsKeyPressed(KeyCode.D))
    //            {
    //                camera.Transform.Position += new Vector3(0.05f, 0.0f, 0.0f);
    //            }
    //            else if (Input.IsKeyPressed(KeyCode.A))
    //            {
    //                camera.Transform.Position -= new Vector3(0.05f, 0.0f, 0.0f);
    //            }

    //            if (Input.IsKeyPressed(KeyCode.W))
    //            {
    //                camera.Transform.Position += new Vector3(0.0f, 0.05f, 0.0f);
    //            }
    //            else if (Input.IsKeyPressed(KeyCode.S))
    //            {
    //                camera.Transform.Position -= new Vector3(0.0f, 0.05f, 0.0f);
    //            }
    //        }

    //        var pos = Transform.Position;
    //        pos.Normalize();
    //        sprite.Color = new Vector4(
    //            Math.Abs(pos.x), Math.Abs(pos.y), Math.Abs(pos.z), 1.0f
    //        );

    //    }

    //    protected override void OnDestroyed()
    //    {
    //        Debug.Info("{0} destroyed!", Id);
    //    }

    //    protected override void OnCollision2DEnter(Collision2DInfo info, Entity other)
    //    {
    //        // var sprite = GetComponent<SpriteRendererComponent>();
    //        //sprite.Color = Color.White;
    //        //Destroy();
    //    }

    //    protected override void OnCollision2DLeave(Collision2DInfo info, Entity other)
    //    {
    //        //var sprite = GetComponent<SpriteRendererComponent>();
    //        //sprite.Color = Vector4.One;
    //    }


    //    private void DestroyAfter(int seconds) {

    //        ThreadManager.Create(() => { 
    //            Thread.Sleep(seconds * 1000);
    //            this.Destroy();
    //        }).Start();
    //    }

    //}
}
