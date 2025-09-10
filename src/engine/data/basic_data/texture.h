#ifndef MITE_DATA_TEXTURE
#define MITE_DATA_TEXTURE

#include "headers/headers.h"
#include "basic_type/handle_type.h"

namespace mite {
/**
 * GPU纹理运行时封装
 * 职责：
 * - 维护纹理采样状态（Wrap/Filter等）
 * - 提供类型安全的绑定接口
 * - 不管理生命周期（由Renderer负责）
 */
class Texture {
 public:
  Texture(const TextureGPUHandle &handle);

  // ---- 元数据访问 ----
  /**
   * 获取GPU句柄
   */
  TextureGPUHandle GetHandle() const
  {
    return m_Handle;
  }
  /**
   * 获取文件路径，用于调试
   */
  const std::string GetPath() const {
    return m_Handle.path;
  }

 private:
  TextureGPUHandle m_Handle;
};

};  // namespace mite

#endif
