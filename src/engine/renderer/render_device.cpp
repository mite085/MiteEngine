#include "render_device.h"

namespace {
// 静态变量隐藏在匿名namespace中
std::mutex &GetDeviceMutex()
{
  static std::mutex mutex;
  return mutex;
}

std::unique_ptr<mite::IRenderDevice> &GetCurrentDevice()
{
  static std::unique_ptr<mite::IRenderDevice> instance = nullptr;
  return instance;
}
}  // namespace

namespace mite {
// ---- 实现静态方法 ----
IRenderDevice &IRenderDevice::Current()
{
  std::lock_guard<std::mutex> lock(GetDeviceMutex());
  if (!GetCurrentDevice()) {
    throw std::runtime_error("No active render device set!");
  }
  return *GetCurrentDevice();
}

void IRenderDevice::SetCurrent(std::unique_ptr<IRenderDevice> device)
{
  std::lock_guard<std::mutex> lock(GetDeviceMutex());
  GetCurrentDevice() = std::move(device);
}
IRenderDevice::IRenderDevice()
{
  // 订阅事件
  m_EventSubscriptions.Subscribe<ModelLoadEvent>(BIND_DISPATCH_FN(OnModelLoaded));
  m_EventSubscriptions.Subscribe<TextureLoadEvent>(BIND_DISPATCH_FN(OnTextureLoaded));
}
};  // namespace mite