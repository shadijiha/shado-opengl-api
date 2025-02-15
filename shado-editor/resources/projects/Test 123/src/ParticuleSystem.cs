using System;
using Shado;

namespace Sandbox
{
    #if false
    public class ParticuleSystem : Entity
    {
        public int numberOfParticles = 10;
        public float minSpeed = 2f;
        public float maxSpeed = 5f;
        public Colour[] particleColors = new Colour[0];
        static Random random = new Random();
        bool hasinit = false;
        Entity particle;
        internal void OnCreate()
        {
            // Emit particles when the scene starts
            EmitParticles();
        }

        internal void OnUpdate(float dt) {
            if (!hasinit) {
                Vector3 randomDirection = new Vector3(
                    (float)random.NextDouble() * 2f - 1f,
                    (float)random.NextDouble() * 2f - 1f,
                    (float)random.NextDouble() * 2f - 1f
                ).normalized;
                float randomSpeed = (float)random.NextDouble() * (maxSpeed - minSpeed) + minSpeed;
                var rb = particle.GetComponent<RigidBody2DComponent>();
                rb.type = RigidBody2DComponent.BodyType.Dynamic;
                rb.ApplyLinearImpulse((randomDirection * randomSpeed).xy, new Vector2(2, 2), false);
                hasinit = true;
            }
        }

        private void EmitParticles()
        {
            for (int i = 0; i < numberOfParticles; i++)
            {
                Vector3 spawnPosition = transform.position;
                transform.scale = new Vector3(0.1f, 0.10f, 0.10f);

                Colour randomColor = GetRandomColor();

                particle = this.Create<ParticuleSystem>(() => new ParticuleSystem());
                particle.AddComponent<SpriteRendererComponent>().colour = randomColor;
                particle.transform.position = spawnPosition;
                particle.AddComponent<RigidBody2DComponent>().type = RigidBody2DComponent.BodyType.Dynamic;
            }
        }

        private Colour GetRandomColor()
        {
            if (particleColors.Length == 0)
            {
                return Colour.White;
            }

            int randomIndex = random.Next(particleColors.Length);
            return particleColors[randomIndex];
        }
    }
#endif
}
