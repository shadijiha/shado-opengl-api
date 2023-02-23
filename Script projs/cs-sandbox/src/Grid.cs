using System;
using Shado;
using Shado.Editor;

namespace Sandbox
{
    public class Test : Entity { 
        public float maxDelta;
        public float moveRate;
        public float tilingFactor = 1.0f;
        private TransformComponent transform;
        private float dir = 1;

        private Texture2D texture;
        public Vector3 test;

        void OnCreate() {
            transform = GetComponent<TransformComponent>();
            texture = Texture2D.Create("Assets/riven2.jpg");
            GetComponent<SpriteRendererComponent>().texture = texture;
        }

        void OnUpdate(float dt) {
            //Vector3 pos = transform.position;
            //pos.x += moveRate * dt * dir;
            //transform.position = pos;

            //if (pos.x > maxDelta || pos.x < -maxDelta) {
            //    dir = -dir;
            //}

            //GetComponent<CircleRendererComponent>().tilingFactor = tilingFactor;
        }
    }

    [EditorTargetType(typeof(Vector3))]
    public class Vector3Editor : Editor {
        protected override void OnEditorDraw()
        {
            Vector3 vec = (Vector3)target;
            Console.WriteLine("Drawing " + target);
        }
    }

    //public class Grid : Entity
    //{
    //    ParticuleSystem system = new ParticuleSystem();
    //    protected override void OnCreate()
    //    {

    //        const float max = 5.0f;
    //        for (float y = -5.0f; y < max; y += 0.5f)
    //        {
    //            for (float x = -5.0f; x < max; x += 0.5f)
    //            {
    //                GridCell GridCell = Create(() => new GridCell(this, x, y));

    //            }
    //         }

    //       /* for (int i = 0; i < 1000; i++) {
    //            ParticuleProps props = new ParticuleProps();
    //            props.position = Vector3.Zero;
    //            props.velocity = Vector3.One;
    //            props.colorBegin = Color.Red;
    //            props.colorEnd = Color.Blue;
    //            props.sizeBegin = 0.1f;
    //            props.sizeEnd = 0.2f;
    //            props.sizeVariation = 0.01f;
    //            props.lifeTime = 1.0f;


    //            system.Emit(props);
    //        }*/
    //    }

    //    protected override void OnUpdate(float dt)
    //    {
    //        //Debug.Info(system.Count());
    //        //system.OnUpdate(dt);
    //    }
    //}

    //class GridCell : Entity {
    //    static readonly Texture2D[] texture = {
    //          new Texture2D(@"assets\riven.png"),
    //          new Texture2D(@"assets\riven2.jpg"),
    //          new Texture2D(@"assets\riven3.png")
    //     };
    //    static Random random = new Random();

    //    float x, y;
    //    Entity parent;
    //    public GridCell(Entity parent, float x, float y) { 
    //        this.x = x;
    //        this.y = y;
    //        this.parent = parent;
    //    }
    //    protected override void OnCreate() {
    //       Transform.Position = parent.Transform.Position + new Vector3(x, y, 0);
    //       Transform.Scale = new Vector3(0.45f, 0.45f, 0);
    //       Tag = $"Grid cell {x}, {y}";

    //        var sprite = AddComponent<SpriteRendererComponent>();
    //        sprite.Color = new Vector4((x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f);
    //        sprite.Texture = texture[random.Next(texture.Length)];
 
    //        var rb = AddComponent<RigidBody2DComponent>();
    //        rb.Type = RigidBody2DComponent.BodyType.DYNAMIC;

    //        AddComponent<BoxCollider2DComponent>();
    //    }

    //    protected override void OnCollision2DEnter(Collision2DInfo info, Entity other)
    //    {
    //        //if (other.Tag.ToLower() == "ground")
    //        //    Destroy();
    //    }
    //}
}
