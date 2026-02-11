using System;
using System.Collections.Generic;
using System.Diagnostics;
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
    /**
     * Original shado script
     */
    public class SScript : IDisposable
    {
        public Test entity { get; set; }
        
        protected string compilerExe = new("D:\\Code\\Projects\\Compiler\\Cs Compile test\\bin\\Release\\net6.0\\Cs Compile test.exe");
        protected StreamWriter? scriptProcessIn;
        protected string scriptPath = @"D:\Code\Projects\shado-java-graphics-api\assets\cmd_script.sscript";
        protected FileSystemWatcher? watcher;
        public CancellationTokenSource TokenSourceource { get; private set; }

        public SScript()
        {
            Init();
        }

        protected void Init()
        {
            TokenSourceource = new();
            watcher = new(Path.GetDirectoryName(scriptPath));
            watcher.Filter = Path.GetFileName(scriptPath);
            watcher.NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.FileName | NotifyFilters.Size;
            
            watcher.Changed += ReloadScript;
            watcher.Created += ReloadScript;

            watcher.EnableRaisingEvents = true;

            Entity? textEntity = Entity.FindEntityByName("sscript content");
            if (textEntity is not null)
                textEntity.GetComponent<TextComponent>().text = File.ReadAllText(scriptPath);
        }
        
        public void Execute()
        {
            RunScriptAsync(compilerExe, scriptPath, TokenSourceource.Token);
        }
        
        protected async Task RunScriptAsync(string compilerExePath, string scriptPath, CancellationToken token)
        {
            try
            {
                var startInfo = new ProcessStartInfo
                {
                    FileName = compilerExePath,
                    Arguments = GetProcessArguments(),
                    RedirectStandardInput = true,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    UseShellExecute = false,
                    CreateNoWindow = true
                };

                var process = new Process { StartInfo = startInfo, EnableRaisingEvents = true };
                process.Start();

                scriptProcessIn = process.StandardInput;

                try
                {
                    var outputTask = Task.Run(async () =>
                    {
                        using var reader = process.StandardOutput;
                        while (await reader.ReadLineAsync(token) is { } line && !token.IsCancellationRequested)
                        {
                            ProcessCommand(line.Trim());
                        }
                    }, token);

                    await outputTask;
                }
                catch (OperationCanceledException e)
                {
                    Log.Info("Cancelling SScript");
                }
                
                scriptProcessIn.Close(); // Close stdin to signal end of input

                Log.Info($"killing process {process.ProcessName}...");
                process.Kill(true);
                
                await process.WaitForExitAsync(token);
                ProcessCommand($"EXIT,{process.ExitCode}");
            }
            catch (Exception e)
            {
                ProcessCommand($"ERROR,{e.Message}");
            }
        }
        
        protected void ReloadScript(object sender, FileSystemEventArgs e) {
            Log.Info("SScript Change detected. Reloading...");
            Dispose();
            Init();
            Execute();
        }

        protected void ProcessCommand(String rawCommand) {
            Log.Info("> " + rawCommand);
            String[] tokens = rawCommand.Trim().Split([",", ";"], StringSplitOptions.TrimEntries | StringSplitOptions.RemoveEmptyEntries);
            String command = tokens[0].ToLower();

            switch (command) {
                case "exit":
                    break;
                case "error":
                    Log.Info(tokens[1]);
                    break;
                case "wait": {
                    var ms = tokens[1];
                    Thread.Sleep(int.Parse(ms));
                    break;
                }
                case "left": {
                    entity.transform.position =
                        entity.transform.position with { x = entity.transform.position.x - 1.0f };
                    break;
                }
                case "right": {
                    entity.transform.position =
                        entity.transform.position with { x = entity.transform.position.x + 1.0f };
                    break;
                }
                case "colour": {
                    float r = float.Parse(tokens[1]) / 255.0f;
                    float g = float.Parse(tokens[2]) / 255.0f;
                    float b = float.Parse(tokens[3]) / 255.0f;
                    entity.GetComponent<SpriteRendererComponent>().colour = new Vector4(r, g, b, 1);
                    break;
                }
                default:
                    Log.Error("Unexpected value: " + command);
                    break;
            }
        }

        public void SendToScript(String message) {
            scriptProcessIn.Write(message + "\n");
            scriptProcessIn.Flush();
        }

        public void Dispose()
        {
            TokenSourceource.Cancel();
            scriptProcessIn?.Dispose();
            watcher?.Dispose();
            TokenSourceource.Dispose();
        }

        protected virtual string GetProcessArguments()
        {
            return $"--filepath \"{scriptPath}\"";
        }
    }

    /**
     * Shado simple script with end keywords 
     */
    public class SimpleScript : SScript
    {
        public SimpleScript()
        {
            Dispose();
            compilerExe = @"java";
            scriptPath = @"D:\Code\Projects\shado-script-simplified\test.simple";
            Init();
        }

        protected override string GetProcessArguments()
        {
            return $@"-jar ""D:\Code\Projects\shado-script-simplified\out\artifacts\shado_script_simplified_jar\shado-script-simplified.jar"" -time -f ""{scriptPath}"" -import-dir ""D:\Code\Projects\shado-script-simplified\stdlib""";
        }
    }
    
    public class Test : Entity
    {
        static Random random = new Random();
        
        public float maxDelta = 3.0f;
        public float moveRate = 1.0f;
        [ShowInEditor] private float dir = 1;

        public Texture2D texture;
        //public Shader shader = Shader.Create("Assets/empty.glsl");
        public float totalDt = 0.0f;
        public Prefab linePrefab;

        private volatile string? textureToLoad = null;

        private List<GridCell> cells = new();

        private SScript sscript;

        protected override void OnCreate()
        {
            //FetchRandomCaitImage();
            sscript = new SimpleScript
            {
                entity = this
            };
            sscript.Execute();
            
            const float max = 5.0f;
            // for (float y = -5.0f; y < max; y += 0.5f)
            // {
            //     for (float x = -5.0f; x < max; x += 0.5f)
            //     {
            //         Entity entity = CreateEmptyEntity();
            //         cells.Add(new GridCell(entity, x, y));
            //         cells[^1].OnCreate();
            //     }
            // }
            
            // texture = new Texture2D("Assets/riven2.jpg");
            // GetComponent<SpriteRendererComponent>()!.tilingFactor = (int)Mathf.Random(2, 10);
            // GetComponent<SpriteRendererComponent>()!.texture = texture;
            //
            // OnCollision2DEnterEvent += (info, other) => {
            //     Log.Info("Collision with {0}", other?.tag);
            // };
            //
            // Entity e = linePrefab.Instantiate(Vector3.one);
            // e.transform.rotation = new Vector3(0, Mathf.DegToRad(30.0f), 0);
            
            sscript.SendToScript("queen");
            sscript.SendToScript("Queen cait from game engine C#");
        }

        protected override void OnUpdate(float dt)
        {
            return;
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

            foreach (var cell in cells)
            {
                cell.OnUpdate(dt);
            }
        }

        protected override void OnDestroy()
        {
            sscript.Dispose();
        }
        
        private WebRequestHelper web = new();
        
        record struct DirListResponse
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
        
        private async void FetchRandomCaitImage()
        {
            await web.MakeRequest("auth/login", WebRequestHelper.RequestMethod.Post, new
            {
                email = "admin@shado.com",
                password = "*********"
            });
            
            await web.MakeRequest("directory/list/Cait", WebRequestHelper.RequestMethod.Get, "", callback: async (content) =>
            {
                // Output the response content
                string responseContent = await content.ReadAsStringAsync();
                DirListResponse? json = JsonSerializer.Deserialize<DirListResponse>(responseContent);
                List<DirListResponse.Data> files = new ();
                foreach (DirListResponse.Data entry in json?.data.Where(e => !e.is_dir && (e.extension == ".png" || e.extension == ".jpg")) ?? [])
                {
                    files.Add(entry);
                }
                
                // random 1
                var chosen = files[random.Next(files.Count)];
                await web.MakeRequest($"file/{web.EncodeUrl(chosen.path)}", WebRequestHelper.RequestMethod.Get, "", callback: async (content) =>
                {
                    byte[] buffer = await content.ReadAsByteArrayAsync();
                    await File.WriteAllBytesAsync($"{Project.current.assetDirectory}/cait_temp" + chosen.extension, buffer);
                    textureToLoad = $"Assets/cait_temp{chosen.extension}";
                });
            });
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
    
    class GridCell {
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

        public GridCell(Entity parent, float x, float y) { 
            this.x = x;
            this.y = y;
            this.parent = parent;
        }

        public void OnCreate() {
            var script = parent.GetComponent<ScriptComponent>();
            //Log.Info("Class: {0}, ID: {1}", script.ClassName, this.ID);
            parent.transform.position = parent.transform.position + new Vector3(x, y, 0);
            parent.transform.scale = new Vector3(0.45f, 0.45f, 0);
            parent.tag = $"Grid cell {x}, {y}";

            var sprite = parent.AddComponent<CircleRendererComponent>();
            sprite.colour = new Vector4((x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f);
            //sprite.colour = Colour.Random();
            //sprite.texture = texture[random.Next(texture.Length)];
 
            var rb = parent.AddComponent<RigidBody2DComponent>();
            parent.AddComponent<BoxCollider2DComponent>();
            rb.type = RigidBody2DComponent.BodyType.Dynamic;
        }

        private volatile bool canJump = true;
        private async Task Jump()
        {
            if (canJump is false)   
                return;
            canJump = false;
            parent.GetComponent<RigidBody2DComponent>().ApplyLinearImpulse(Vector2.up * 3.0f, false);
            await Task.Delay(2000);
            canJump = true;
        }

        public void OnUpdate(float dt)
        {
            
            //GetComponent<SpriteRendererComponent>().colour = Colour.FromHSL(angle % 360, 100, 100);
            angle += dt;

            if (Input.IsKeyDown(KeyCode.Space))
            {
                Jump();
            }
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

    class WebRequestHelper
    {
        public enum RequestMethod
        {
            Get, Post
        }
        
        private CookieContainer cookieContainer = new();
        private HttpClientHandler handler;
        private HttpClient httpClient;

        public WebRequestHelper()
        {
            handler = new HttpClientHandler
            {
                CookieContainer = cookieContainer,
                UseCookies = true,
                ServerCertificateCustomValidationCallback = (message, certificate, chain, sslPolicyErrors) => true
            };
            httpClient = new HttpClient(handler);
        }
        
        public async Task MakeRequest<T>(string url, RequestMethod method, T? body = null, Func<HttpContent, Task>? callback = null) where T : class
        {
            Log.Info("Requesting {0}", url);
            var requestUrl = $"https://cloud.shadijiha.com/apinest/{url}"; // Replace with the desired URL
            string jsonBody = JsonSerializer.Serialize(body);
            var bodyStr = new StringContent(jsonBody, Encoding.UTF8, "application/json");

            HttpResponseMessage response = method switch
            {
                RequestMethod.Get => await httpClient.GetAsync(requestUrl),
                RequestMethod.Post => await httpClient.PostAsync(requestUrl, bodyStr),
                _ => throw new Exception(),
            };
            
            response.EnsureSuccessStatusCode();

            if (callback is not null)
                await callback(response.Content);
        }

        public string EncodeUrl(string url) => HttpUtility.UrlEncode(url).Replace("+", "%20");
    }
}
