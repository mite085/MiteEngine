#ifndef MITE_IMGUI_BACKEND_H
#define MITE_IMGUI_BACKEND_H

#include "ui_core/ui_backend.h"
#include "ui_imgui_localization_renderer.h"
#include "ui_imgui_input_adapter.h"
#include "ui_imgui_style_adapter.h"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
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

  // ==================== UIBackend接口实现 ====================
  bool Initialize(void *window) override;
  void Shutdown() override;
  void BeginFrame() override;
  void EndFrame() override;
  void ProcessInputEvent(Event &event) override;
  void SetDisplaySize(int width, int height) override;
  glm::ivec2 GetDisplaySize() const override;
  void SetFramebufferScale(float scaleX, float scaleY) override;
  glm::vec2 GetFramebufferScale() const override;
  void SetMouseCaptured(bool captured) override;
  bool IsMouseCaptured() const override;
  void SetMouseCursorVisible(bool visible) override;
  bool IsMouseCursorVisible() const override;
  void CreateDeviceObjects() override;
  void DestroyDeviceObjects() override;
  void Render() override;
  const char *GetBackendName() const override;
  void ApplyUIStyle(std::shared_ptr<UIStyle> newStyle) override;
  void ApplyLanguaged(const std::string &oldLanguage, const std::string &newLanguage) override;

  // ==================== ImGui特定方法 ====================
  void SetWindow(GLFWwindow *window);
  GLFWwindow *GetWindow() const;

  // 输入管理（委托给InputAdapter）
  ImGuiInputAdapter& GetInputAdapter() {
    return *m_InputAdapter;
  }

 private:
  // ==================== 内部方法 ====================
  // 初始化ImGui上下文
  bool InitializeImGuiContext();

  // 初始化平台后端
  bool InitializePlatformBackend();

  // 初始化渲染器后端
  bool InitializeRendererBackend();

  // 成员变量
  GLFWwindow *m_Window;                               // GLFW窗口句柄
  bool m_MouseCaptured;                               // 是否捕获鼠标
  bool m_MouseCursorVisible;                          // 鼠标指针是否可见
  glm::ivec2 m_DisplaySize;                           // 显示尺寸
  glm::vec2 m_FramebufferScale;                       // 帧缓冲缩放
  double m_Time = 0.0f;                               // 时间跟踪
  std::unique_ptr<ImGuiStyleAdapter> m_StyleAdapter;  // 样式适配器
  std::unique_ptr<ImGuiInputAdapter> m_InputAdapter;  // 输入适配器

  Logger m_Logger;                        // 日志系统
};
}  // namespace mite

#endif  // MITE_IMGUI_BACKEND_H
