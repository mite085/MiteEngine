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
  /**
   * G-Buffer纹理索引枚举(format通过getTextureFormat(index)定义)
   * 其中：
   * NPRParameters：rampThreshold色阶阈值、rampSmoothness色阶平滑度、specularSize高光尺寸、outlineWidth描边宽度
   * NPRColors：shadowTint.rgb阴影色调、rimPower边缘光衰减
   */
  enum GBufferIndex {
    GBUFFER_WORLDPOS_DEPTH = 0,    // World position(xyz)+Depth(w，线性深度)(RGBA32F)
    GBUFFER_BASECOLOR_MATTYPE,     // BaseColor(rgb)+MaterialType(a，标志位)(RGBA16F)
    GBUFFER_METALLICROUGHNESS_AO,  // MetallicRoughness(xy)+AO(z，w保留)(RGBA16F)
    GBUFFER_NORMAL_SCALE,          // Normal(xyz) + NormalScale(w)(RGBA16F)
    GBUFFEE_EMISSION_ALPHA,        // Emission(rgb) + Alpha(a)(RGBA16F)
    GBUFFER_NPR_PARAM,             // NPRParameters(RGBA16F)
    GBUFFER_NPR_COLOR,             // NPRColors(RGBA16F)
    COUNT  // G-Buffer纹理总数（Enum尾部，自动生成占位符，不包含任何信息）
  };

 public:
  GBuffer();
  ~GBuffer();

  /**
   * 初始化G-Buffer
   * @param width G-Buffer宽度
   * @param height G-Buffer高度
   * @return 初始化是否成功
   */
  bool create(uint32_t width, uint32_t height);
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
  RuntimeTexturePtr getTexture(GBufferIndex index) const;
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
  TextureFormat getTextureFormat(GBufferIndex index) const;


 private:
  std::shared_ptr<FrameBuffer> m_framebuffer;                     // G-Buffer帧缓冲
  uint32_t m_width = 0;                                           // G-Buffer宽度
  uint32_t m_height = 0;                                          // G-Buffer高度
  bool m_isValid = false;                                         // G-Buffer是否有效
};

// 智能指针别名
using GBufferPtr = std::shared_ptr<GBuffer>;
}  // namespace mite

#endif  // MITE_GBUFFER_H
