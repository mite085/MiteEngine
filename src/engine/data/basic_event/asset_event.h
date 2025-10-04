#ifndef MITE_ASSTE_EVENT
#define MITE_ASSTE_EVENT

#include "basic_type/asset_type.h"
#include "basic_type/handle_type.h"
#include "basic_type/material_type.h"
#include "subscription_group.h"

namespace mite {
/**
 * 模型创建事件
 * 职责：委托RendererDevice创建GPU资源
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
 * 外部纹理加载事件
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

/**
 * 运行时纹理创建事件
 * 职责：委托RendererDevice创建GPU资源
 */
class RuntimeTextureCreateEvent : public Event {
 public:
  RuntimeTextureCreateEvent(std::shared_ptr<TextureCreateInfo> createInfo,
                            std::function<void(TextureGPUHandle)> callback)
      : m_CreateInfo(createInfo), m_Callback(callback)
  {
  }
  std::shared_ptr<TextureCreateInfo> GetTextureCreateInfo()
  {
    return m_CreateInfo;
  }
  std::function<void(TextureGPUHandle)> GetCallback()
  {
    return m_Callback;
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new RuntimeTextureCreateEvent(m_CreateInfo, m_Callback);
  }

 private:
  // 回调函数，负责在Device端将Handle传回事件的发送者
  std::function<void(TextureGPUHandle)> m_Callback;
  std::shared_ptr<TextureCreateInfo> m_CreateInfo;
};

/**
 * 运行时纹理销毁申请事件
 *职责：委托RendererDevice销毁GPU资源
 */
class RuntimeTextureDestroyRequestEvent : public Event {
 public:
  RuntimeTextureDestroyRequestEvent(TextureGPUHandle handle) : m_Handle(handle) {}
  TextureGPUHandle GetTextureGPUHandle()
  {
    return m_Handle;
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new RuntimeTextureDestroyRequestEvent(m_Handle);
  }

 private:
  TextureGPUHandle m_Handle;
};
}  // namespace mite

#endif
