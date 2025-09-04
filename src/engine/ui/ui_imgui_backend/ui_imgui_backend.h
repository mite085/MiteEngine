#ifndef MITE_IMGUI_BACKEND_H
#define MITE_IMGUI_BACKEND_H

#include "ui_core/ui_backend.h"
#include "ui_imgui_localization_renderer.h"

struct GLFWwindow;

namespace mite {
/**
 * @brief ImGui后端实现
 *
 * 初始化：由UISystem::Initialize()负责
 *
 * 1. 创建ImGui后端
 *   m_Backend = std::make_unique<ImGuiBackend>();
 *
 * 2. 设置GLFW窗口（从Window模块获取）
 *   GLFWwindow *window = // 获取GLFW窗口 ;
 *   static_cast<ImGuiBackend *>(m_Backend.get())->SetWindow(window);
 *
 * 3. 初始化后端
 *   if (!m_Backend->Initialize(this)) {
 *     Logger::Get().Error("Failed to initialize UI backend");
 *     return false;
 *   }
 */
class ImGuiBackend : public UIBackend {
 public:
  ImGuiBackend();
  ~ImGuiBackend() override;

  // UIBackend接口实现
  bool Initialize() override;
  void Shutdown() override;
  void BeginFrame() override;
  void EndFrame() override;
  void ProcessInputEvent(const Event &event) override;
  void SetDisplaySize(int width, int height) override;
  std::pair<int, int> GetDisplaySize() const override;
  void SetFramebufferScale(float scaleX, float scaleY) override;
  std::pair<float, float> GetFramebufferScale() const override;
  void SetMouseCaptured(bool captured) override;
  bool IsMouseCaptured() const override;
  void SetMouseCursorVisible(bool visible) override;
  bool IsMouseCursorVisible() const override;
  void CreateDeviceObjects() override;
  void DestroyDeviceObjects() override;
  void Render() override;
  const char *GetBackendName() const override;

  // ImGui特定方法
  void SetWindow(GLFWwindow *window);
  GLFWwindow *GetWindow() const;

  // 样式管理（委托给StyleAdapter）
  void ApplyDarkStyle();
  void ApplyLightStyle();
  void ApplyStyle(const std::string &styleName);
  void ApplyCustomStyle(const ImGuiStyle &style);
  ImGuiStyleAdapter &GetStyleAdapter()
  {
    return *m_InputAdapter;
  }

  // 输入管理（委托给InputAdapter）
  ImGuiInputAdapter &GetInputAdapter();

 private:
  // 初始化ImGui上下文
  bool InitializeImGuiContext();

  // 初始化平台后端
  bool InitializePlatformBackend();

  // 初始化渲染器后端
  bool InitializeRendererBackend();

  // 处理语言切换事件
  void OnLanguageChanged(LanguageChangedEvent &event);

  // 成员变量
  GLFWwindow *m_Window;                               // GLFW窗口句柄
  bool m_MouseCaptured;                               // 是否捕获鼠标
  bool m_MouseCursorVisible;                          // 鼠标指针是否可见
  std::pair<int, int> m_DisplaySize;                  // 显示尺寸
  std::pair<float, float> m_FramebufferScale;         // 帧缓冲缩放
  double m_Time;                                      // 时间跟踪
  std::unique_ptr<ImGuiStyleAdapter> m_StyleAdapter;  // 样式适配器
  std::unique_ptr<ImGuiInputAdapter> m_InputAdapter;  // 输入适配器

  Logger m_Logger;                        // 日志系统
  SubscriptionGroup m_SubscriptionGroup;  // 事件订阅系统
};
}  // namespace mite

#endif  // MITE_IMGUI_BACKEND_H
