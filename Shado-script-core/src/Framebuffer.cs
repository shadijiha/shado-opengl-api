using System;

namespace Shado
{
    public enum FramebufferTextureFormat : int
    {
        None = 0,

        // Color
        RGBA8,
        RED_INTEGER,

        // Depth/stencil
        DEPTH24STENCIL8,

        // Defaults
        Depth = DEPTH24STENCIL8
    };

    public struct FramebufferSpecification
    {
        public uint width { get; set; }
        public uint height { get; set; }
        public uint samples;
        public FramebufferTextureFormat[] attachments;
        public bool swapChainTarget;

        public FramebufferSpecification(uint width, uint height, uint samples = 1, bool swapChainTarget = false) {
            this.width = width;
            this.height = height;
            this.samples = samples;
            this.swapChainTarget = swapChainTarget;
            this.attachments = new[] {
                FramebufferTextureFormat.RGBA8, FramebufferTextureFormat.RED_INTEGER,
                FramebufferTextureFormat.DEPTH24STENCIL8
            };
        }
    }

    // public class Framebuffer : IDisposable
    // {
    //     internal IntPtr nativePtr;  // C++ Frambuffer ptr
    //     
    //     public Framebuffer(FramebufferSpecification specs) {
    //         nativePtr = InternalCalls.Framebuffer_Create(specs.width, specs.height, specs.samples, specs.attachments, specs.swapChainTarget);
    //     }
    //
    //     public uint GetColorAttachmentRendererID(uint index = 0) {
    //         return InternalCalls.Framebuffer_GetColorAttachmentRendererID(nativePtr, index);
    //     }
    //     
    //     public void Invalidate() {
    //         InternalCalls.Framebuffer_Invalidate(nativePtr);
    //     }
    //     
    //     public void Bind() {
    //         InternalCalls.Framebuffer_Bind(nativePtr);
    //     }
    //     
    //     public void Unbind() {
    //         InternalCalls.Framebuffer_Unbind(nativePtr);
    //     }
    //     
    //     public void Resize(uint width, uint height) {
    //         InternalCalls.Framebuffer_Resize(nativePtr, width, height);
    //     }
    //     
    //     public void ClearAttachment(uint attachmentIndex, int value) {
    //         InternalCalls.Framebuffer_ClearAttachment(nativePtr, attachmentIndex, value);
    //     }
    //
    //     public void Dispose() {
    //         InternalCalls.Framebuffer_Destroy(nativePtr);
    //     }
    // }
}