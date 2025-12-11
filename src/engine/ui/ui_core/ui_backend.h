#ifndef MITE_UI_BACKEND_H
#define MITE_UI_BACKEND_H

#include "subscription_group.h"

namespace mite {

// 前向声明
class UIStyle;

/**
 * @brief UI后端抽象接口
 *
 * 定义UI系统与渲染后端之间的契约，支持多种UI后端实现
 */
class UIBackend {
 public:
  virtual ~UIBackend() = default;
  virtual bool Initialize(void *window) = 0;
  virtual void Shutdown() = 0;

  // ==================== 渲染接口与事件处理 ====================
  /**
   * @brief 开始UI帧
   * @param menuBarCallback 菜单栏绘制的回调函数
   * @note 由于Imgui的限制，菜单栏绘制必须在DockSpace创建之前完成，
   * 而DockSpace的创建**应当**是作为BeginFrame的一部分，所以只能以回调函数输入来处理
   */
  virtual void BeginFrame(std::function<void()> menuBarCallback) = 0;

  /**
   * @brief 结束UI帧
   */
  virtual void EndFrame() = 0;

  // ==================== 参数接口 ====================
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

  // ==================== 样式语言管理 ====================
  /**
   * @brief 样式管理
   */
  virtual void ApplyUIStyle(std::shared_ptr<UIStyle>) = 0;

  /**
   * @brief 语言管理
   */
  virtual void ApplyLanguaged(const std::string &oldLanguageCode, const std::string &newLanguageCode) = 0;
};

}  // namespace mite

#endif  // MITE_UI_BACKEND_H
