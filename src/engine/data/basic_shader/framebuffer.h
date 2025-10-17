#ifndef MITE_FRAMEBUFFER_H
#define MITE_FRAMEBUFFER_H

#include "basic_type/framebuffer_type.h"

namespace mite {
/**
 * @brief 帧缓冲类，封装OpenGL FBO功能
 *
 * 职责：
 * 1. 管理帧缓冲对象(FBO)的生命周期
 * 2. 管理颜色/深度/模板附件（作为RuntimeTexture）
 * 3. 提供渲染目标绑定接口
 * 4. 支持多渲染目标(MRT)和HDR(为未来扩展保留)
 */
class FrameBuffer {
 public:
  /**
   * @brief 构造函数
   * @param spec 帧缓冲规格
   */
  explicit FrameBuffer(const FrameBufferSpec &spec);
  ~FrameBuffer();

  // 禁止拷贝和赋值
  FrameBuffer(const FrameBuffer &) = delete;
  FrameBuffer &operator=(const FrameBuffer &) = delete;

  glm::vec2 GetSize() const {
    return {m_Spec.width, m_Spec.height};
  }

  // ==================== 核心接口 ====================
  /**
   * @brief 重新设置帧缓冲大小
   * @param width 新宽度
   * @param height 新高度
   */
  void Resize(uint32_t width, uint32_t height);
  /**
   * @brief 绑定帧缓冲为当前渲染目标
   */
  void Bind() const;
  /**
   * @brief 解绑帧缓冲(绑定回默认帧缓冲)
   */
  void Unbind() const;
  /**
   * @brief 获取帧缓冲对象ID
   * @return 帧缓冲对象ID
   */
  uint32_t GetID() const;

  // ==================== 纹理附件接口 ====================
  /**
   * @brief 获取颜色附件纹理ID
   * @param index 颜色附件索引(默认为0)
   * @return 纹理ID
   */
  RuntimeTexturePtr GetColorAttachment(uint32_t index = 0) const;
  /**
   * @brief 获取深度附件运行时纹理
   * @return 深度纹理共享指针（如果存在）
   */
  RuntimeTexturePtr GetDepthAttachment() const;
  /**
   * @brief 获取模板附件运行时纹理
   * @return 模板纹理共享指针（如果存在）
   */
  RuntimeTexturePtr GetStencilAttachment() const;
  /**
   * @brief 获取所有颜色附件
   * @return 颜色附件映射表
   */
  const std::unordered_map<uint32_t, RuntimeTexturePtr> &GetColorAttachments() const
  {
    return m_ColorAttachments;
  }

  // ==================== 完整性检查 ====================
  /**
   * @brief 获取帧缓冲规格
   * @return 帧缓冲规格引用
   */
  const FrameBufferSpec &GetSpecification() const
  {
    return m_Spec;
  }

  /**
   * @brief 检查帧缓冲是否完整
   * @return 如果完整返回true，否则返回false
   */
  bool IsComplete() const;

 private:
  /**
   * @brief 初始化帧缓冲和附件
   */
  void Invalidate();

  /**
   * @brief 清理帧缓冲资源
   */
  void Release();

 private:
  uint32_t m_RendererID = 0;  // 帧缓冲对象ID
  FrameBufferSpec m_Spec;     // 帧缓冲规格

  // 附件运行时纹理ID映射表
  // key: 附件索引
  // value: 纹理ID
  std::unordered_map<uint32_t, RuntimeTexturePtr> m_ColorAttachments;
  RuntimeTexturePtr m_DepthAttachment = nullptr;
  RuntimeTexturePtr m_StencilAttachment = nullptr;
};

}  // namespace mite

#endif  // MITE_FRAMEBUFFER_H