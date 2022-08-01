using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Shado;
using System.Threading;

namespace Sandbox
{
    public class Ground : Entity
    {
        volatile int collisions;


        protected override void OnCreate()
        {
            ThreadManager.Create(() => { 
                Thread.Sleep(2 * 1000);
                Debug.Info(collisions);
            }).Start();
        }

        protected override void OnCollision2DEnter(Collision2DInfo info, Entity other) {
            collisions++;
        }

    }
}
