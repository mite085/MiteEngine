#ifndef MITE_GBUFFER_H
#define MITE_GBUFFER_H

#include "basic_data/runtime_texture.h"
#include "framebuffer.h"

namespace mite {
/**
 * G-Buffer类 - 管理延迟渲染的几何缓冲区
 *
 * 职责：
 * 1. 创建和管理G-Buffer的所有纹理附件
 * 2. 提供G-Buffer纹理的访问接口
 * 3. 管理G-Buffer帧缓冲对象
 * 4. 处理G-Buffer的调整大小和验证
 */
class GBuffer {
 public:
  // GBuffer纹理数量(与RuntimeTextureType中GBuffer定义保持一致)
  static constexpr size_t TEXTURE_COUNT = 7;
  // 纹理类型到附件索引的映射
  static const std::map<RuntimeTextureType, uint32_t> TextureTypeToIndex;
  // 获取所有GBuffer纹理类型
  static const std::vector<RuntimeTextureType> &GetTextureTypes();
  // 获取GBuffer纹理类型的字符串名称(用于着色器绑定Gbuffer)
  static const char *GetTextureTypeName(RuntimeTextureType type);

 public:
  GBuffer();
  ~GBuffer();

  /**
   * 初始化G-Buffer
   * @param width G-Buffer宽度
   * @param height G-Buffer高度
   * @return 初始化是否成功
   */
  bool create();
  /**
   * 清理G-Buffer资源
   */
  void cleanup();
  /**
   * 调整G-Buffer大小
   * @param newWidth 新宽度
   * @param newHeight 新高度
   * @return 调整是否成功
   */
  bool resize(uint32_t newWidth, uint32_t newHeight);
  /**
   * 验证G-Buffer完整性
   * @return G-Buffer是否完整可用
   */
  bool validate() const;

  // 访问器
  RuntimeTexturePtr getTexture(RuntimeTextureType index) const;
  std::shared_ptr<FrameBuffer> getFramebuffer() const;
  int getWidth() const;
  int getHeight() const;
  bool isValid() const;

  /**
   * 绑定G-Buffer为当前渲染目标
   */
  void bind() const;

  /**
   * 解绑G-Buffer
   */
  void unbind() const;

 private:
  /**
   * 创建G-Buffer帧缓冲规格
   * @return 帧缓冲规格
   */
  FrameBufferSpec createFrameBufferSpec() const;

  /**
   * 获取指定索引的纹理格式
   * @param index 纹理索引
   * @return 纹理格式
   */
  TextureFormat getTextureFormat(RuntimeTextureType index) const;

 private:
  std::shared_ptr<FrameBuffer> m_framebuffer;  // G-Buffer帧缓冲
  uint32_t m_width = 1;                        // G-Buffer宽度
  uint32_t m_height = 1;                       // G-Buffer高度
  bool m_isValid = false;                      // G-Buffer是否有效
};

// 智能指针别名
using GBufferPtr = std::shared_ptr<GBuffer>;
}  // namespace mite

#endif  // MITE_GBUFFER_H
