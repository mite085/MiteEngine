#ifndef MITE_ASSET_MODEL_LOADER
#define MITE_ASSET_MODEL_LOADER

#include "headers/headers.h"
#include "assimp/scene.h"
#include "basic_type/asset_type.h"

namespace mite {
/**
 * 模型加载器（纯数据解析，不涉及GPU资源创建）
 * 职责：
 * 1. 解析模型文件（FBX/OBJ/GLTF等）为引擎中间格式
 * 2. 转换顶点数据为统一布局
 * 3. 提取材质和层级关系
 */
class ModelLoader {
 public:
  /**
   * 加载模型文件
   * @param path 模型文件路径
   * @param flipUVs 是否翻转UV垂直坐标（适配OpenGL坐标系）
   * @return 包含模型元数据和所有子网格数据的结构体
   * @throws std::runtime_error 当模型加载失败时抛出异常
   */
  static std::shared_ptr<ModelAsset> LoadModel(const std::string &path, bool flipUVs = true);

 private:
  // 处理Assimp的Mesh数据
  static MeshData ProcessMesh(const aiMesh *aiMesh, const aiScene *scene);

  // 处理顶点布局描述（供Renderer模块使用）
  static VertexLayout GenerateVertexLayout(const aiMesh *aiMesh);

  // 计算模型的包围盒
  static void CalculateBoundingBox(const std::vector<MeshData> &subMeshes,
                                   glm::vec3 &outMin,
                                   glm::vec3 &outMax);
  // 提取材质路径列表
  static std::vector<std::string> ExtractMaterialPaths(const aiScene *scene);
};

/**
 * 模型创建事件
 * 职责：委托RendererDevice创建GPU资源
 */
class ModelLoadEvent : public Event {
 public:
  ModelLoadEvent(std::shared_ptr<ModelAsset> asset) : m_Asset(asset) {}

  std::shared_ptr<ModelAsset> GetModelAsset()
  {
    return m_Asset;
  }

  EVENT_CLASS_TYPE(SCENE_LOADED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new ModelLoadEvent(m_Asset);
  }

 private:
  std::shared_ptr<ModelAsset> m_Asset;
};
};  // namespace mite

#endif
