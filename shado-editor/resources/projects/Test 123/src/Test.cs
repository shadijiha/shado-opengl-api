using System;
using System.Threading.Tasks;
using Shado;
using Shado.Editor;

namespace Sandbox
{ 
    public class Test : Entity
    {
        public float maxDelta = 3.0f;
        public float moveRate = 1.0f;
        [ShowInEditor] private float dir = 1;

        public Texture2D texture;
        //public Shader shader = Shader.Create("Assets/empty.glsl");
        public float totalDt = 0.0f;
        public Prefab linePrefab;
        //private ParticuleSystem particuleSystem;

        //public ShadoEvent events;

        protected override void OnCreate() {
            const float max = 0.0f;
            for (float y = -5.0f; y < max; y += 0.5f)
            {
                for (float x = -5.0f; x < max; x += 0.5f)
                {
                    // TODO: Change the Entity.Create to following signature
                    // Create<T>(param object[] ctorArgs)
                    // The reason for that is because right now it is not possible to invode 
                    // internal methods such as AddComponent in the .ctor
                    //GridCell gridCell = Create<GridCell>(() => new GridCell(this, x ,y));
                }
            }

            texture = new Texture2D("Assets/riven2.jpg");
            GetComponent<SpriteRendererComponent>().tilingFactor = (int)Mathf.Random(2, 10);
            GetComponent<SpriteRendererComponent>().texture = texture;
            
            OnCollision2DEnterEvent += (info, other) => {
                Log.Info("Collision with {0}", other.tag);
            };

            Entity e = linePrefab.Instantiate(Vector3.one);
            e.transform.rotation = new Vector3(0, Mathf.DegToRad(30.0f), 0);
        }

        protected override void OnUpdate(float dt)
        {
            totalDt += dt;
            //if (GetComponent<SpriteRendererComponent>().texture != texture)
            //    GetComponent<SpriteRendererComponent>().texture = texture;
            Vector3 pos = transform.position;
            
            pos.x += moveRate * 2 * dt * dir;
            transform.position = pos;

            if (pos.x > maxDelta || pos.x <= -maxDelta)
            {
                dir *= -1;
            }

            if (Input.IsKeyDown(KeyCode.L)) {
                Log.Info("Loading new scene...");
                Scene.LoadScene("raytracing.shadoscene");
            }
            
            if (Input.IsKeyDown(KeyCode.C)) {
               GetComponent<RigidBody2DComponent>().ApplyLinearImpulse(Vector2.one, false);
            }
        }
        
    }

    public class TestCamera : Entity {

        public float moveDelta = 5.0f;
        private CameraComponent? camera;
        public Entity? camera2;

        protected override void OnCreate() { 
            if (HasComponent<CameraComponent>())
                camera = GetComponent<CameraComponent>();
            //camera2 = FindEntityByName("camera2");
        }

        protected override void OnUpdate(float dt) {
            
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

    public class TextIncrementor : Entity
    {
        TextComponent? text;
        Entity? square;
        public float fieldPrefabTest = 0.0f;

        protected override void OnCreate() {
            text = GetComponent<TextComponent>();
            text.text = "0";
            square = FindEntityByName("Square");
        }

        protected override void OnUpdate(float dt) {
            text.text = square.transform.position.x + "";
        }
        
    }
    
    class GridCell : Entity {
        static readonly Texture2D[] texture = {
           new(@"Assets\riven.png"),
           new(@"Assets\riven2.jpg"),
           new(@"Assets\riven3.png")
        };
        static Random random = new Random();
        static uint collisions = 0;

        internal float x, y;
        internal Entity parent;
        float angle = 0.01f;

        public GridCell() { }
        public GridCell(Entity parent, float x, float y) { 
            this.x = x;
            this.y = y;
            this.parent = parent;
        }

        protected override void OnCreate() {
            var script = GetComponent<ScriptComponent>();
            //Log.Info("Class: {0}, ID: {1}", script.ClassName, this.ID);
            transform.position = parent.transform.position + new Vector3(x, y, 0);
            transform.scale = new Vector3(0.45f, 0.45f, 0);
            tag = $"Grid cell {x}, {y}";

            var sprite = AddComponent<SpriteRendererComponent>();
            //sprite.colour = new Vector4((x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f);
            sprite.colour = Colour.Random();
            //sprite.texture = texture[random.Next(texture.Length)];
 
            var rb = AddComponent<RigidBody2DComponent>();
            AddComponent<BoxCollider2DComponent>();
            rb.type = RigidBody2DComponent.BodyType.Dynamic;
        }

        protected override void OnUpdate(float dt)
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
