using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using Shado;
using Shado.Editor;

namespace Sandbox
{ 
    public class Test : Entity
    {
        public float maxDelta = 3.0f;
        public float moveRate = 1.0f;
        public float tilingFactor = 1.0f;
        //private TransformComponent transform;
        private float dir = 1;

        public Texture2D texture = Texture2D.Create("Assets/riven2.jpg");
        public Colour colourTest;
        public Vector3 debug_size = Vector3.one;
        public float angle = 0.0f;
        public Shader shader = Shader.Create("Assets/empty.glsl");
        public float totalDt = 0.0f;
        private Test other;
        public Prefab linePrefab;
        public Prefab squarePrefab;
        //private ParticuleSystem particuleSystem;
        public Prefab boidPrefab;

        public ShadoEvent events;

        void OnCreate() {
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
                    //GridCell gridCell = Create<GridCell>(() => new GridCell(this, x ,y));
                }
            }

            //particuleSystem = Create<ParticuleSystem>();
            //particuleSystem.OnCreate();
            
            if (linePrefab)
            {
                //Entity e = linePrefab.Instantiate(Vector3.one);
            }
            
            // BOids
            for(int i = 0; i < 1; i++) {
                //Entity e = boidPrefab.Instantiate(Vector3.Random(-1.0f, 1.0f));
            }

            if (false) {
                new AudioTest()
                    .AddNote(new AudioTest.Note(AudioTest.Note.C, 2))
                    .AddNote(new AudioTest.Note(AudioTest.Note.C, 3))
                    .AddNote(new AudioTest.Note(AudioTest.Note.G, 3))
                    .AddNote(new AudioTest.Note(AudioTest.Note.C, 4))
                    .AddNote(new AudioTest.Note(AudioTest.Note.E, 4))
                    .AddNote(new AudioTest.Note(AudioTest.Note.G, 4))
                    .AddNote(new AudioTest.Note(AudioTest.Note.ASharp, 5))
                    .AddNote(new AudioTest.Note(AudioTest.Note.C, 5))
                    .AddNote(new AudioTest.Note(AudioTest.Note.D, 5))
                    .AddNote(new AudioTest.Note(AudioTest.Note.E, 5))
                    .AddNote(new AudioTest.Note(AudioTest.Note.FSharp, 5))
                    .AddNote(new AudioTest.Note(AudioTest.Note.G, 5))
                    .AddNote(new AudioTest.Note(AudioTest.Note.A, 6))
                    .AddNote(new AudioTest.Note(AudioTest.Note.ASharp, 6))
                    .AddNote(new AudioTest.Note(AudioTest.Note.B, 6))
                    .AddNote(new AudioTest.Note(AudioTest.Note.C, 6))
                    .SaveFile();
            }
        }

        void OnUpdate(float dt)
        {
            //particuleSystem.OnUpdate(dt);   
            totalDt += dt;
            GetComponent<SpriteRendererComponent>().colour = Colour.Random(); //colourTest;
            GetComponent<SpriteRendererComponent>().texture = texture;
            Vector3 pos = transform.position;
            
            pos.x += moveRate * 2 * dt * dir;
            transform.position = pos;

            if (pos.x > maxDelta || pos.x <= -maxDelta)
            {
                dir *= -1;
            }

            if (Input.IsKeyDown(KeyCode.K) && other is null) {
                //other = Create<Test>();
            } 
            if (Input.IsKeyDown(KeyCode.L)) {
                Log.Info("Loading new scene...");
                Scene.LoadScene("raytracing.shadoscene");
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

    class Boid : Entity
    {
        static List<Boid> boids = new List<Boid>();
        Vector3 linearVelocity = Vector3.Random(-0.5f, 0.5f);
        void OnCreate() {
            boids.Add(this);
            
            if (HasComponent<RigidBody2DComponent>())
                throw new Exception("RigidBody2DComponent cannot exist in the Boid class");
        }

        void OnUpdate(float dt) {
            var b = this;
            Vector3 vector1 = rule1(b); // Boids flock to the center of mass
            Vector3 vector2 = rule2(b); // Boids avoid other boids
            Vector3 vector3 = rule3(b); // Boids try to match the speed of other boids

            Vector3 finalVector = vector1 + vector2 + vector3;
            b.linearVelocity += finalVector * dt; // Adjust direction and speed
            b.transform.position += b.linearVelocity; // Update the position to the new position
        }

        Vector3 rule1(Boid b) {
            Vector3 pC = Vector3.zero; // Number of dimensions can change
            foreach (var b2 in boids) {
                if (b != b2) {
                    pC += b2.transform.position;
                }
            }

            pC = pC / (boids.Count - 1);
            Vector3 result = (pC - b.transform.position) / 1.0f; // 0.5% towards the percieved center
            return result;
        }

        Vector3 rule2(Boid b) {
            double distance = 0.1f; // Threshold of distance between boids
            Vector3 result = Vector3.zero;
            foreach (var b2 in boids) {
                if (b != b2) // Ignore duplicate boids
                    if (Vector3.Distance(b.transform.position, b2.transform.position) < distance)
                        result -= (b2.transform.position - b.transform.position);   
            }
            return result;
        }

        Vector3 rule3(Boid b) {
            Vector3 pV = Vector3.zero; // Number of dimensions can change
            foreach (var b2 in boids) {
                if (b != b2)
                    pV += b2.linearVelocity;
            }

            pV = pV / (boids.Count - 1);
            Vector3 result = (pV - b.linearVelocity) / 10.0f; // 0.5% towards the percieved center
            return result;
        }

    }
    
    class AudioTest
    {
        static int samples = 88200 * 4;
        static int samplesPerSecond = 44100;
        short[] a, b, c, d;
        
        public AudioTest() {
            a = new short[samples / 4];
            b = new short[samples / 4];
            c = new short[samples / 4];
            d = new short[samples / 4];
        }
        
        public AudioTest AddNote(Note note) {
            double aNatural = note.Frequency;
            double ampl = 10000.0f / 8.0f;
            double perfect = 1.5;
            double concert = 1.498307077;
            double freq = aNatural * perfect;
            for (int i = 0; i < samples / 4; i++) {
                double t = (double)i / (double)samplesPerSecond;
                short s = (short)(ampl * (Math.Sin(t * freq * 2.0 * Math.PI)));
                a[i] += s;
            }
            freq = aNatural * concert;
            for (int i = 0; i < samples / 4; i++) {
                double t = (double)i / (double)samplesPerSecond;
                short s = (short)(ampl * (Math.Sin(t * freq * 2.0 * Math.PI)));
                b[i] += s;
            }
            for (int i = 0; i < samples / 4; i++) {
                double t = (double)i / (double)samplesPerSecond;
                short s = (short)(ampl * (Math.Sin(t * freq * 2.0 * Math.PI) + Math.Sin(t * freq * perfect * 2.0 * Math.PI)));
                c[i] += s;
            }
            for (int i = 0; i < samples / 4; i++) {
                double t = (double)i / (double)samplesPerSecond;
                short s = (short)(ampl * (Math.Sin(t * freq * 2.0 * Math.PI) + Math.Sin(t * freq * concert * 2.0 * Math.PI)));
                d[i] += s;
            }

            return this;
        }

        public void SaveFile() {
            FileStream stream = new FileStream("test.wav", FileMode.Create);
             BinaryWriter writer = new BinaryWriter(stream);
             int RIFF = 0x46464952;
             int WAVE = 0x45564157;
             int formatChunkSize = 16;
             int headerSize = 8;
             int format = 0x20746D66;
             short formatType = 1;
             short tracks = 1;
             int samplesPerSecond = 44100;
             short bitsPerSample = 16;
             short frameSize = (short)(tracks * ((bitsPerSample + 7)/8));
             int bytesPerSecond = samplesPerSecond * frameSize;
             int waveSize = 4;
             int data = 0x61746164;
             int samples = 88200 * 4;
             int dataChunkSize = samples * frameSize;
             int fileSize = waveSize + headerSize + formatChunkSize + headerSize + dataChunkSize;
             writer.Write(RIFF);
             writer.Write(fileSize);
             writer.Write(WAVE);
             writer.Write(format);
             writer.Write(formatChunkSize);
             writer.Write(formatType);
             writer.Write(tracks); 
             writer.Write(samplesPerSecond);
             writer.Write(bytesPerSecond);
             writer.Write(frameSize);
             writer.Write(bitsPerSample); 
             writer.Write(data);
             writer.Write(dataChunkSize);
             double aNatural = 220.0;
             double ampl = 10000;
             double perfect = 1.5;
             double concert = 1.498307077;
             double freq = aNatural * perfect;
             for (int i = 0; i < a.Length; i++) {
                double t = (double)i / (double)samplesPerSecond;
                short s = (short)(ampl * (Math.Sin(t * freq * 2.0 * Math.PI)));
                writer.Write(a[i]);
             }
             freq = aNatural * concert;
             for (int i = 0; i < b.Length; i++) {
                double t = (double)i / (double)samplesPerSecond;
                short s = (short)(ampl * (Math.Sin(t * freq * 2.0 * Math.PI)));
                writer.Write(b[i]);
             }
             for (int i = 0; i < c.Length; i++) {
                double t = (double)i / (double)samplesPerSecond;
                short s = (short)(ampl * (Math.Sin(t * freq * 2.0 * Math.PI) + Math.Sin(t * freq * perfect * 2.0 * Math.PI)));
                writer.Write(c[i]);
             }
             for (int i = 0; i < d.Length; i++) {
                double t = (double)i / (double)samplesPerSecond;
                short s = (short)(ampl * (Math.Sin(t * freq * 2.0 * Math.PI) + Math.Sin(t * freq * concert * 2.0 * Math.PI)));
                writer.Write(d[i]);
             }
             writer.Close();
             stream.Close();
        }
        
        public class Note {
            public const double C = 130.8f;
            public const double CSharp = 138.6f;
            public const double D = 146.8f;
            public const double DSharp = 155.6f;
            public const double E = 164.8f;
            public const double F = 174.6f;
            public const double FSharp = 185.0f;
            public const double G = 196.0f;
            public const double GSharp = 207.7f;
            public const double A = 110.0f;
            public const double ASharp = 116.5f;
            public const double B = 123.5f;
            
            private const int baseOctave = 3;
            private int currentOctave;
            private double currentNote;

            public double Frequency => currentNote * Math.Pow(2, currentOctave - baseOctave);
            public Note(double note, int octave = baseOctave) {
                currentNote = note;
                currentOctave = octave;
            } 
        }
    }
}
