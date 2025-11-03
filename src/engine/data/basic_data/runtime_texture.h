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
  bool initialize(RuntimeTextureType type, int width, int height, TextureFormat format, TextureTarget target);
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
  bool resize(int newWidth, int newHeight);

  // 访问器
  TextureGPUHandle getHandle() const { return m_handle; }
  RuntimeTextureType getType() const { return m_type; }
  int getWidth() const { return m_width; }
  int getHeight() const { return m_height; }
  TextureFormat getFormat() const { return m_format; }
  bool isValid() const { return m_handle.apiHandle != 0; }

 private:
  TextureGPUHandle m_handle{0};                                  // GPU纹理句柄
  RuntimeTextureType m_type = RuntimeTextureType::RenderTarget;  // 纹理类型
  int m_width = 0;                                               // 纹理宽度
  int m_height = 0;                                              // 纹理高度
  TextureFormat m_format = TextureFormat::RGBA8;                 // 纹理格式
  TextureTarget m_Target = TextureTarget::TEXTURE_2D;			 // 纹理目标
};

// 智能指针别名
using RuntimeTexturePtr = std::shared_ptr<RuntimeTexture>;
}  // namespace mite

#endif  // MITE_RUNTIME_TEXTURE_H
