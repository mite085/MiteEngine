#ifndef MITE_RUNTIME_TEXTURE_H
#define MITE_RUNTIME_TEXTURE_H

#include "basic_type/handle_type.h"

namespace mite {
/**
 * 运行时纹理类 - 专门用于G-Buffer、ShadowMap等渲染目标
 * 与TextureAsset不同，这类纹理不从外部文件加载，而是程序运行时动态创建
 */
class RuntimeTexture {
 public:
  RuntimeTexture() = default;
  ~RuntimeTexture();

  /**
   * 初始化运行时纹理
   * @param type 纹理类型
   * @param width 纹理宽度
   * @param height 纹理高度
   * @param format 纹理格式
   * @return 初始化是否成功
   */
  bool initialize(RuntimeTextureType type, uint32_t width, uint32_t height,
                  TextureFormat format, TextureTarget target,
                  uint32_t arrayLayers);
  /**
   * 清理纹理资源
   */
  void cleanup();
  /**
   * 调整纹理尺寸（用于窗口大小变化等场景）
   * @param newWidth 新宽度
   * @param newHeight 新高度
   * @return 调整是否成功
   */
  bool resize(uint32_t newWidth, uint32_t newHeight);

  // 访问器
  TextureGPUHandle GetHandle() const { return m_Handle; }
  RuntimeTextureType GetType() const { return m_Type; }
  uint32_t GetWidth() const { return m_Width; }
  uint32_t GetHeight() const { return m_Height; }
  TextureFormat GetFormat() const { return m_Format; }
  TextureTarget GetTarget() const { return m_Target; }
  uint32_t GetArrayLayers() const { return m_ArrayLayers; }
  bool IsValid() const { return m_Handle.apiHandle != 0; }

 private:
  TextureGPUHandle m_Handle{0};  // GPU纹理句柄
  RuntimeTextureType m_Type = RuntimeTextureType::RenderTarget;  // 纹理类型
  uint32_t m_Width = 0;                                          // 纹理宽度
  uint32_t m_Height = 0;                                         // 纹理高度
  TextureFormat m_Format = TextureFormat::RGBA8;                 // 纹理格式
  TextureTarget m_Target = TextureTarget::TEXTURE_2D;            // 纹理目标
  uint32_t m_ArrayLayers = 1;  // 纹理层数（如果是数组纹理）
};

// 智能指针别名
using RuntimeTexturePtr = std::shared_ptr<RuntimeTexture>;
}  // namespace mite

#endif  // MITE_RUNTIME_TEXTURE_H
