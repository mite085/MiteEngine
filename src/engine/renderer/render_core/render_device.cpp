#include "render_device.h"
#include "render_opengl/opengl_device.h"

namespace mite {
// ---- 实现静态方法 ----
RenderDevice::RenderDevice()
{
  // 订阅模型与纹理加载完成事件
  // Immediate同步模式：
  // OnModelLoaded是使用加载好的模型数据创建GPU资源，
  // 而OpenGL上下文是线程相关的，一般只在主线程使用。至于模型本身数据的加载则可以异步
  m_EventSubscriptions.SubscribeImmediate<ModelLoadEvent>(BIND_DISPATCH_FN(OnModelLoaded),
                                                          EventPriority::Normal);
  m_EventSubscriptions.SubscribeImmediate<TextureLoadEvent>(BIND_DISPATCH_FN(OnTextureLoaded),
                                                            EventPriority::Normal);

  // 订阅运行时纹理加载和销毁事件
  // Immediate同步模式，理由同上
  m_EventSubscriptions.SubscribeImmediate<RuntimeTextureCreateEvent>(
      BIND_DISPATCH_FN(OnRuntimeTextureCreate), EventPriority::Normal);
  m_EventSubscriptions.SubscribeImmediate<RuntimeTextureDestroyRequestEvent>(
      BIND_DISPATCH_FN(OnRuntimeTextureDestroyRequest), EventPriority::Normal);
}
};  // namespace mite