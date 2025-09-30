#ifndef MITE_FRAMEBUFFER_H
#define MITE_FRAMEBUFFER_H

#include "basic_type/framebuffer_type.h"

namespace mite {
/**
 * @brief 帧缓冲类，封装OpenGL FBO功能
 *
 * 职责：
 * 1. 管理帧缓冲对象(FBO)的生命周期
 * 2. 管理颜色/深度/模板附件
 * 3. 提供渲染目标绑定接口
 * 4. 支持多渲染目标(MRT)和HDR(为未来扩展保留)
 */
class FrameBuffer {
 public:
  using Ptr = std::shared_ptr<FrameBuffer>;

  /**
   * @brief 构造函数
   * @param spec 帧缓冲规格
   */
  explicit FrameBuffer(const FrameBufferSpec &spec);
  ~FrameBuffer();

  // 禁止拷贝和赋值
  FrameBuffer(const FrameBuffer &) = delete;
  FrameBuffer &operator=(const FrameBuffer &) = delete;

  glm::uvec2 GetSize() const {
    return {m_Spec.width, m_Spec.height};
  }

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

  /**
   * @brief 获取颜色附件纹理ID
   * @param index 颜色附件索引(默认为0)
   * @return 纹理ID
   */
  uint32_t GetColorAttachmentID(uint32_t index = 0) const;

  /**
   * @brief 获取深度附件纹理ID
   * @return 纹理ID，如果没有深度附件则返回0
   */
  uint32_t GetDepthAttachmentID() const;

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

  // 附件纹理ID映射表
  // key: 附件索引(对于颜色附件)或附件类型(对于深度/模板附件)
  // value: 纹理ID
  std::unordered_map<uint32_t, uint32_t> m_ColorAttachments;
  uint32_t m_DepthAttachment = 0;  // 深度附件纹理ID
};

}  // namespace mite

#endif  // MITE_FRAMEBUFFER_H