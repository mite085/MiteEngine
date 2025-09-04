#ifndef MITE_UI_BACKEND_H
#define MITE_UI_BACKEND_H

#include "event/event.h"
#include <functional>
#include <memory>

namespace mite {

// 前向声明
class UISystem;

/**
 * @brief UI后端抽象接口
 *
 * 定义UI系统与渲染后端之间的契约，支持多种UI后端实现
 */
class UIBackend {
 public:
  virtual ~UIBackend() = default;

  /**
   * @brief 初始化后端
   * @param uiSystem UI系统实例
   * @return 是否初始化成功
   */
  virtual bool Initialize(UISystem *uiSystem) = 0;

  /**
   * @brief 开始UI帧
   */
  virtual void BeginFrame() = 0;

  /**
   * @brief 结束UI帧
   */
  virtual void EndFrame() = 0;

  /**
   * @brief 处理输入事件
   * @param event 输入事件
   */
  virtual void ProcessInputEvent(const Event &event) = 0;

  /**
   * @brief 设置显示尺寸
   * @param width 宽度
   * @param height 高度
   */
  virtual void SetDisplaySize(int width, int height) = 0;

  /**
   * @brief 获取显示尺寸
   * @return 包含宽度和高度的pair
   */
  virtual std::pair<int, int> GetDisplaySize() const = 0;

  /**
   * @brief 设置帧缓冲缩放
   * @param scaleX 水平缩放
   * @param scaleY 垂直缩放
   */
  virtual void SetFramebufferScale(float scaleX, float scaleY) = 0;

  /**
   * @brief 获取帧缓冲缩放
   * @return 包含水平和垂直缩放的pair
   */
  virtual std::pair<float, float> GetFramebufferScale() const = 0;

  /**
   * @brief 设置是否捕获鼠标
   * @param captured 是否捕获
   */
  virtual void SetMouseCaptured(bool captured) = 0;

  /**
   * @brief 获取是否捕获鼠标
   * @return 是否捕获
   */
  virtual bool IsMouseCaptured() const = 0;

  /**
   * @brief 设置是否显示鼠标光标
   * @param visible 是否显示
   */
  virtual void SetMouseCursorVisible(bool visible) = 0;

  /**
   * @brief 获取是否显示鼠标光标
   * @return 是否显示
   */
  virtual bool IsMouseCursorVisible() const = 0;

  /**
   * @brief 创建后端特定的渲染资源
   */
  virtual void CreateDeviceObjects() = 0;

  /**
   * @brief 销毁后端特定的渲染资源
   */
  virtual void DestroyDeviceObjects() = 0;

  /**
   * @brief 渲染UI
   */
  virtual void Render() = 0;

  /**
   * @brief 获取后端名称（用于调试）
   */
  virtual const char *GetBackendName() const = 0;
};

}  // namespace mite

#endif  // MITE_UI_BACKEND_H
