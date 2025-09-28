#ifndef MITE_DATA_TEXTURE
#define MITE_DATA_TEXTURE

#include "headers/headers.h"
#include "basic_type/handle_type.h"

namespace mite {
/**
 * GPU纹理运行时封装（提供的功能太少，暂时弃用）
 */
class Texture {
 public:
  Texture(const TextureGPUHandle &handle) : m_Handle(handle) {}

  // ---- 元数据访问 ----
  /**
   * 获取GPU句柄
   */
  TextureGPUHandle GetHandle() const
  {
    return m_Handle;
  }

 private:
  TextureGPUHandle m_Handle;
};

};  // namespace mite

#endif
