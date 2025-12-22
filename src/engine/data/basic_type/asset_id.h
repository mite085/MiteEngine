#ifndef MITE_ASSET_ID
#define MITE_ASSET_ID

#include "uuid/mite_uuid.h"

namespace mite {
// 类型安全的资产ID包装器
template <typename Tag>
class TypedAssetID {
 private:
  UUID id_;

 public:
  TypedAssetID() : id_() {}  // 默认无效ID
  explicit TypedAssetID(const UUID &id) : id_(id) {}
  explicit TypedAssetID(const char *str) : id_(UUIDGenerator::Generate(str)) {}

  const UUID &GetUUID() const { return id_; }
  bool IsValid() const { return !id_.is_nil(); }

  // 比较操作
  bool operator==(const TypedAssetID &other) const { return id_ == other.id_; }
  bool operator!=(const TypedAssetID &other) const { return id_ != other.id_; }
  bool operator<(const TypedAssetID &other) const { return id_ < other.id_; }

  // 哈希支持
  struct Hash {
    size_t operator()(const TypedAssetID &id) const {
      return std::hash<UUID>()(id.id_);
    }
  };
};

// 具体的资产类型标签
struct TextureAssetTag {};
struct MaterialAssetTag {};
struct ModelAssetTag {};

// 类型安全的资产ID
using TextureAssetID = TypedAssetID<TextureAssetTag>;
using MaterialAssetID = TypedAssetID<MaterialAssetTag>;
using ModelAssetID = TypedAssetID<ModelAssetTag>;
}  // namespace mite

#endif
