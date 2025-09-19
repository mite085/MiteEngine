#ifndef MITE_SCENE_CORE_COMPONENT_SNAPSHOT
#define MITE_SCENE_CORE_COMPONENT_SNAPSHOT

#include "component.h"
#include "basic_type/snapshot.h"

namespace mite {

/**
 * @brief 组件快照基类
 */
class ComponentSnapshot : public ISnapshot {
 public:
  /**
   * @brief 构造函数
   * @param entityId 实体ID
   * @param componentType 组件类型
   */
  ComponentSnapshot(Entity entityId, std::type_index componentType)
      : ISnapshot()  // 调用基类构造函数，自动设置时间戳
        ,
        m_entityId(entityId),
        m_componentType(componentType)
  {
  }

  virtual ~ComponentSnapshot() = default;

  // ISnapshot接口实现
  void Apply() override;
  void Revert() override;
  size_t GetMemoryUsage() const override;
  const char *GetDescription() const override;

  /**
   * @brief 获取关联的实体ID
   */
  Entity GetEntity() const
  {
    return m_entityId;
  }

  /**
   * @brief 获取组件类型
   */
  std::type_index GetComponentType() const
  {
    return m_componentType;
  }

  /**
   * @brief 创建组件快照
   */
  template<typename ComponentT>
  static std::unique_ptr<ComponentSnapshot> Create(const ComponentT &component);

 protected:
  virtual void SerializeState() = 0;
  virtual void DeserializeState() = 0;

  Entity m_entityId;
  std::type_index m_componentType;
  std::vector<uint8_t> m_serializedData;
  size_t m_memoryUsage = 0;
};

/**
 * @brief 模板化的组件快照实现
 */
template<typename ComponentT> class TypedComponentSnapshot : public ComponentSnapshot {
 public:
  TypedComponentSnapshot(Entity entityId, const ComponentT &component)
      : ComponentSnapshot(entityId, typeid(ComponentT)), m_componentData(component)
  {
  }

  // ISnapshot接口实现
  void Apply() override
  {
    if (auto scene = GetScene()) {
      if (auto comp = scene->GetComponent<ComponentT>(m_entityId)) {
        *comp = m_componentData;
      }
    }
  }

  void Revert() override
  {
    Apply();
  }

  size_t GetMemoryUsage() const override
  {
    return sizeof(ComponentT);
  }

  const char *GetDescription() const override
  {
    return typeid(ComponentT).name();
  }

 private:
  ComponentT m_componentData;
};

template<typename ComponentT>
std::unique_ptr<ComponentSnapshot> ComponentSnapshot::Create(const ComponentT &component)
{
  return std::make_unique<TypedComponentSnapshot<ComponentT>>(component.GetEntity(), component);
}

}  // namespace mite::scene

#endif  // MITE_SCENE_CORE_COMPONENT_SNAPSHOT
