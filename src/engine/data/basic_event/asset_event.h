#ifndef MITE_ASSTE_EVENT
#define MITE_ASSTE_EVENT

#include "basic_type/asset_type.h"
#include "basic_type/handle_type.h"
#include "basic_type/material_type.h"
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
  ModelLoadEvent(std::shared_ptr<ModelSourceData> source, std::shared_ptr<ModelAsset> asset)
      : m_Source(source), m_Asset(asset)
  {
  }
  std::shared_ptr<ModelSourceData> GetModelSourceData()
  {
    return m_Source;
  }
  std::shared_ptr<ModelAsset> GetModelAsset()
  {
    return m_Asset;
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_ASSET)
  Event *Clone() const override
  {
    return new ModelLoadEvent(m_Source, m_Asset);
  }

 private:
  std::shared_ptr<ModelSourceData> m_Source;
  std::shared_ptr<ModelAsset> m_Asset;
};

/**
 * 材质创建事件
 * 职责：委托MaterialSystem创建材质实例
 */
class MaterialLoadedEvent : public Event {
 public:
  explicit MaterialLoadedEvent(const MaterialSourceData &sourceData,
                               std::shared_ptr<MaterialAsset> asset)
      : m_SourceData(sourceData), m_Asset(asset)
  {
  }

  const MaterialSourceData &GetSourceData() const
  {
    return m_SourceData;
  }
  std::shared_ptr<MaterialAsset> GetMaterialAsset()
  {
    return m_Asset;
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_ASSET)
  Event *Clone() const override
  {
    return new MaterialLoadedEvent(m_SourceData, m_Asset);
  }

 private:
  MaterialSourceData m_SourceData;
  std::shared_ptr<MaterialAsset> m_Asset;
};

/**
 * 纹理创建事件
 * 职责：委托RendererDevice创建GPU资源
 */
class TextureLoadEvent : public Event {
 public:
  TextureLoadEvent(std::shared_ptr<TextureSourceData> source, std::shared_ptr<TextureAsset> asset)
      : m_Source(source), m_Asset(asset)
  {
  }
  std::shared_ptr<TextureSourceData> GetTextureSourceData()
  {
    return m_Source;
  }
  std::shared_ptr<TextureAsset> GetTextureAsset()
  {
    return m_Asset;
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_ASSET)
  Event *Clone() const override
  {
    return new TextureLoadEvent(m_Source, m_Asset);
  }

 private:
  std::shared_ptr<TextureSourceData> m_Source;
  std::shared_ptr<TextureAsset> m_Asset;
};
}  // namespace mite

#endif
