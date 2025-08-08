#ifndef MITE_ASSET_TYPE
#define MITE_ASSET_TYPE

#include "handle_type.h"
#include "uuid/mite_uuid.h"
#include <glm/glm.hpp>

namespace mite {
// --- 基础类型别名 ---
using AssetID = uuids::uuid;  // 资源唯一标识符（用UUID生成）

// ------------------------ 纹理相关 ------------------------

// 纹理元数据（描述纹理的属性和来源）
struct TextureMetadata {
  std::string path;                             // 资源路径（用于热重载识别）
  TextureFormat format = TextureFormat::RGBA8;  // 数据格式
  int width = 0;                                // 宽度（像素）
  int height = 0;                               // 高度
  int channels = 4;                             // 颜色通道数（RGB=3, RGBA=4）
  bool isHDR = false;                           // 是否是HDR纹理
};

// 纹理数据
// 存储内容为：uint8_t[]纹理数组，void (*)(uint8_t *)析构方法
struct TetxureData {
  std::unique_ptr<uint8_t[], void (*)(uint8_t *)> textureData;
};
// ------------------------ 模型/网格相关 ------------------------

// 子网格数据（API无关的几何数据）
struct MeshData {
  std::vector<uint8_t> vertexData;  // 原始顶点字节流
  std::vector<uint32_t> indices;    // 索引数据
  VertexLayout layout;              // 顶点结构描述
  uint32_t materialIndex = 0;       // 关联的材质索引

  glm::vec3 boundingBoxMin = glm::vec3(FLT_MAX);  // 子网格局部包围盒
  glm::vec3 boundingBoxMax = glm::vec3(-FLT_MAX);
};

// 模型元数据
struct ModelMetadata {
  std::string path;
  std::vector<std::string> materialPaths;         // 材质路径引用
  glm::vec3 boundingBoxMin = glm::vec3(FLT_MAX);  // 模型包围盒
  glm::vec3 boundingBoxMax = glm::vec3(-FLT_MAX);
};

// ------------------------ 资源句柄 --------------------
// 纹理逻辑句柄
struct TextureAsset {
  AssetID id;  // 唯一标识符
  TextureMetadata metadata;
  TetxureData textureData;

  TextureGPUHandle handle;
};

// 模型逻辑句柄
struct ModelAsset {
  AssetID id;
  ModelMetadata metadata;
  std::vector<MeshData> subMeshData;  // 子网格集合

  std::vector<MeshGPUHandle> subMeshHandles; // 子网格GPU句柄
};

};  // namespace mite

#endif
