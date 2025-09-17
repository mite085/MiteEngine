#ifndef MITE_ASSTE_EVENT
#define MITE_ASSTE_EVENT

#include "basic_type/asset_type.h"
#include "basic_type/handle_type.h"
#include "headers/headers.h"
#include "subscription_group.h"

namespace mite {
/**
 * 模型创建事件
 * 职责：委托RendererDevice创建GPU资源
 *
 */
class ModelLoadEvent : public Event {
 public:
  ModelLoadEvent(std::shared_ptr<ModelSourceData> source, std::shared_ptr<ModelGPUHandle> &hanle)
      : m_Source(source), m_Handle(hanle)
  {
  }
  std::shared_ptr<ModelSourceData> GetModelSourceData()
  {
    return m_Source;
  }
  std::shared_ptr<ModelGPUHandle> &GetModelGPUHandle()
  {
    return m_Handle;
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_ASSET)
  Event *Clone() const override
  {
    return new ModelLoadEvent(m_Source, m_Handle);
  }

 private:
  std::shared_ptr<ModelSourceData> m_Source;
  std::shared_ptr<ModelGPUHandle> &m_Handle;
};

/**
 * 纹理创建事件
 * 职责：委托RendererDevice创建GPU资源
 */
class TextureLoadEvent : public Event {
 public:
  TextureLoadEvent(std::shared_ptr<TextureSourceData> source,
                   std::shared_ptr<TextureGPUHandle> &handle)
      : m_Source(source), m_Handle(handle)
  {
  }
  std::shared_ptr<TextureSourceData> GetTextureSourceData()
  {
    return m_Source;
  }
  std::shared_ptr<TextureGPUHandle> &GetTextureHandle()
  {
    return m_Handle;
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_ASSET)
  Event *Clone() const override
  {
    return new TextureLoadEvent(m_Source, m_Handle);
  }

 private:
  std::shared_ptr<TextureSourceData> m_Source;
  std::shared_ptr<TextureGPUHandle> &m_Handle;
};
}  // namespace mite

#endif
