#ifndef MITE_UI_BACKEND_H
#define MITE_UI_BACKEND_H

#include "headers/headers.h"
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
   */
  virtual void BeginFrame() = 0;

  /**
   * @brief 结束UI帧
   */
  virtual void EndFrame() = 0;

  // ==================== 参数接口 ====================
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
  virtual glm::ivec2 GetDisplaySize() const = 0;

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
  virtual glm::vec2 GetFramebufferScale() const = 0;

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

  // ==================== 渲染资源管理（暂时没有用到） ====================
  /**
   * @brief 创建后端特定的渲染资源
   */
  virtual void CreateDeviceObjects() = 0;

  /**
   * @brief 销毁后端特定的渲染资源
   */
  virtual void DestroyDeviceObjects() = 0;

  /**
   * @brief 获取后端名称（用于调试）
   */
  virtual const char *GetBackendName() const = 0;


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
