using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using Shado;
using Shado.Editor;
using System.Drawing.Imaging;
using System.Threading;
using System.Web;

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

        private volatile string? textureToLoad = null;
        //private ParticuleSystem particuleSystem;

        //public ShadoEvent events;

        protected override void OnCreate()
        {
            //fetchCait();
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

            if (textureToLoad is not null)
            {
                GetComponent<SpriteRendererComponent>().texture = new Texture2D(textureToLoad);
                Log.Info(GetComponent<SpriteRendererComponent>().texture.ToString());
                textureToLoad = null;
            }
        }

        static CookieContainer cookieContainer = new CookieContainer();
        static HttpClientHandler handler = new HttpClientHandler
        {
            CookieContainer = cookieContainer,
            UseCookies = true, // Enable cookies
            //AutomaticDecompression = DecompressionMethods.GZip | DecompressionMethods.Deflate, // Optional: handle compressed responses
            ServerCertificateCustomValidationCallback = (message, certificate, chain, sslPolicyErrors) => true
        };
        static HttpClient httpClient = new HttpClient(handler);
        static Random random = new Random();
        
        private async Task makeRequest<T>(string url, string method, T? body = null, Func<HttpContent, Task>? callback = null) where T : class
        {
            Log.Info("Requesting {0}", url);
            var requestUrl = $"https://cloud.shadijiha.com/apinest/{url}"; // Replace with the desired URL
            string jsonBody = JsonSerializer.Serialize(body);
            var bodyStr = new StringContent(jsonBody, Encoding.UTF8, "application/json");

            HttpResponseMessage response = null;
            switch (method)
            {
                case "post":
                    response = await httpClient.PostAsync(requestUrl, bodyStr);
                    break;
                case "get":
                    response = await httpClient.GetAsync(requestUrl);
                    break;
                default:
                    throw new Exception();
            }
            
            response.EnsureSuccessStatusCode();

            if (callback is not null)
                await callback(response.Content);
        }
        
        record DirListResponse
        {
            public string status { get; set; }
            public string parent { get; set; }
            public Data[] data {get; set; }
            
            public record Data
            {
                public string name { get; set; }
                public string path { get; set; }
                public bool is_dir { get; set; }
                
                public string extension { get; set; }
            }
        }
        private async void fetchCait()
        {
            await makeRequest("auth/login", "post", new
            {
                email = "example@shado.com",
                password = "*******"
            });
            
            await makeRequest("directory/list/Cait", "get", "", callback: async (content) =>
            {
                
                // Output the response content
                string responseContent = await content.ReadAsStringAsync();
                DirListResponse json = JsonSerializer.Deserialize<DirListResponse>(responseContent);
                List<DirListResponse.Data> files = new ();
                foreach (DirListResponse.Data entry in json.data.Where(e => !e.is_dir && (e.extension == ".png" || e.extension == ".jpg")))
                {
                    files.Add(entry);
                }
                
                // random 1
                var chosen = files[random.Next(files.Count)];
                await makeRequest($"file/{HttpUtility.UrlEncode(chosen.path).Replace("+", "%20")}", "get", "", callback: async (content) =>
                {
                    byte[] buffer = await content.ReadAsByteArrayAsync();
                    File.WriteAllBytes("D:\\Code\\Projects\\shado-opengl-api\\shado-editor\\resources\\projects\\Test 123\\Assets\\cait_temp" + chosen.extension, buffer);
                    textureToLoad = $"Assets/cait_temp{chosen.extension}";
                    //GetComponent<SpriteRendererComponent>().texture = new Texture2D($"Assets/cait_temp{chosen.extension}");
                    //Log.Info(GetComponent<SpriteRendererComponent>().texture.ToString());
                });
            });
            
            // // Access the cookies
            // var cookies = cookieContainer.GetCookies(new Uri(requestUrl));
            // Console.WriteLine("\nStored Cookies:");
            // foreach (Cookie cookie in cookies)
            // {
            //     Console.WriteLine($"{cookie.Name} = {cookie.Value}");
            // }
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
