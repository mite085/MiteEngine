#include "component_type.h"

namespace mite {

// ȫ���������ID������
std::atomic<ComponentType::TypeId> s_NextTypeId{1};

// �������ע���
std::unordered_map<std::type_index, ComponentType> s_TypeRegistry;
std::mutex s_RegistryMutex;

ComponentType::TypeId ComponentType::GetNextTypeId()
{
  return s_NextTypeId.fetch_add(1, std::memory_order_relaxed);
}

template<typename T> ComponentType ComponentType::Get()
{
  static_assert(std::is_class<T>::value, "Component type must be a class type");

  const std::type_index typeIndex = typeid(T);

  // ����Ƿ���ע��
  {
    std::lock_guard lock(s_RegistryMutex);
    auto it = s_TypeRegistry.find(typeIndex);
    if (it != s_TypeRegistry.end()) {
      return it->second;
    }
  }

  // ע��������
  const TypeId id = GetNextTypeId();
  const char *name = typeid(T).name();  // ����ʹ��demangle��ȡ���Ѻõ�����
  const size_t size = sizeof(T);

  ComponentType newType(id, typeIndex, name, size);

  {
    std::lock_guard lock(s_RegistryMutex);
    s_TypeRegistry.emplace(typeIndex, newType);
  }

  return newType;
}
};
