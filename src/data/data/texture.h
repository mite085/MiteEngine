#ifndef MITE_BASIC_TYPE_TEXTURE
#define MITE_BASIC_TYPE_TEXTURE

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

  // ---- 核心接口 ----
  // TODO: Bind操作全权交付给渲染队列进行，此处不应当持有Bind和Set方法。

  void Bind(uint32_t slot) const;
  //void Unbind() const;

  // ---- 状态设置 ----
  void SetWrapMode(TextureWrapMode mode);
  void SetFilterMode(TextureFilterMode mode);
  void GenerateMipmaps();

  // ---- 元数据访问 ----
  TextureGPUHandle GetHandle() const
  {
    return handle_;
  }

 private:
  TextureGPUHandle handle_;
};

/**
 * 纹理绑定事件
 * 职责：委托RendererDevice绑定纹理
 */
class TextureBindEvent : public Event {
 public:
  TextureBindEvent(TextureGPUHandle tex, uint32_t slot) : m_Texture(tex), m_Slot(slot) {}

  TextureGPUHandle GetHandle()
  {
    return m_Texture;
  }
  uint32_t GetSlot()
  {
    return m_Slot;
  }

  EVENT_CLASS_TYPE(SCENE_LOADED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new TextureBindEvent(m_Texture, m_Slot);
  }

 private:
  TextureGPUHandle m_Texture;
  uint32_t m_Slot;
};

/**
 * 纹理包装模式修改事件
 * 职责：委托RendererDevice修改纹理
 */
class TextureWrapModeEvent : public Event {
 public:
  TextureWrapModeEvent(TextureGPUHandle tex, TextureWrapMode mode) : m_Texture(tex), m_Mode(mode) {}

  TextureGPUHandle GetHandle()
  {
    return m_Texture;
  }
  TextureWrapMode GetMode() {
    return m_Mode;
  }

  EVENT_CLASS_TYPE(SCENE_LOADED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new TextureWrapModeEvent(m_Texture, m_Mode);
  }

 private:
  TextureGPUHandle m_Texture;
  TextureWrapMode m_Mode;
};

/**
 * 纹理过滤模式修改事件
 * 职责：委托RendererDevice修改纹理
 */
class TextureFilterModeEvent : public Event {
 public:
  TextureFilterModeEvent(TextureGPUHandle tex, TextureFilterMode mode)
      : m_Texture(tex), m_Mode(mode)
  {
  }

  TextureGPUHandle GetHandle()
  {
    return m_Texture;
  }
  TextureFilterMode GetMode()
  {
    return m_Mode;
  }

  EVENT_CLASS_TYPE(SCENE_LOADED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new TextureFilterModeEvent(m_Texture, m_Mode);
  }

 private:
  TextureGPUHandle m_Texture;
  TextureFilterMode m_Mode;
};

/**
 * 纹理Mipmap生成事件
 * 职责：委托RendererDevice生成Mipmap
 */
class TextureGenerateMipmapsEvent : public Event {
 public:
  TextureGenerateMipmapsEvent(TextureGPUHandle tex) : m_Texture(tex) {}

  TextureGPUHandle GetHandle()
  {
    return m_Texture;
  }

  EVENT_CLASS_TYPE(SCENE_LOADED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new TextureGenerateMipmapsEvent(m_Texture);
  }

 private:
  TextureGPUHandle m_Texture;
};

};  // namespace mite

#endif
