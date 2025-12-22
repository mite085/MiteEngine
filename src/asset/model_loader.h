#ifndef MITE_ASSET_MODEL_LOADER
#define MITE_ASSET_MODEL_LOADER

#include "basic_type/asset_type.h"
#include "asset_cache.h"

struct aiScene;
struct aiMesh;

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
   * @brief LoadGLTFModel 加载GLTF模型到缓存
   * @param path 模型文件路径
   * @param flipUVs 是否翻转UV垂直坐标（适配OpenGL坐标系）
   * @param generateLODs 是否生成多级LOD
   * @param lodLevels LOD级别配置（每个级别的简化比例）
   */
  static ModelAssetID LoadGLTFModel(ModelCache &modelCache,
                                    MaterialCache &materialCache,
                                    TextureCache &textureCache,
                                    const std::string &path,
                                    bool flipUVs = false,
                                    bool generateLODs = false,
                                    const std::vector<float> &lodLevels = {
                                        1.0f, 0.5f, 0.25f, 0.1f});
  /**
   * 加载OBJ模型到缓存
   * @param path 模型文件路径
   * @param flipUVs 是否翻转UV垂直坐标（适配OpenGL坐标系）
   * @param generateLODs 是否生成多级LOD
   * @param lodLevels LOD级别配置（每个级别的简化比例）
   */
  static ModelAssetID LoadObjModel(ModelCache &modelCache,
                                   MaterialCache &materialCache,
                                   TextureCache &textureCache,
                                   const std::string &path,
                                   bool flipUVs = true,  // OBJ通常需要翻转UV
                                   bool generateLODs = false,
                                   const std::vector<float> &lodLevels = {
                                       1.0f, 0.5f, 0.25f, 0.1f});

  /**
   * @brief 通用模型加载（PLY、FBX等格式调用，暂未针对性优化）
   * @param path 模型文件路径
   * @param flipUVs 是否翻转UV垂直坐标（适配OpenGL坐标系）
   * @param generateLODs 是否生成多级LOD
   * @param lodLevels LOD级别配置（每个级别的简化比例）
   * @return 包含模型元数据和所有子网格数据的结构体
   * @throws std::runtime_error 当模型加载失败时抛出异常
   */
  static ModelAssetID LoadModel(ModelCache &modelCache,
                                MaterialCache &materialCache,
                                TextureCache &textureCache,
                                const std::string &path,
                                bool flipUVs = false,
                                bool generateLODs = false,
                                const std::vector<float> &lodLevels = {1.0f, 0.5f, 0.25f, 0.1f});

 private:
  /**
   * 核心加载实现
   */
  static ModelAssetID LoadModelInternal(ModelCache &modelCache,
                                        MaterialCache &materialCache,
                                        TextureCache &textureCache,
                                        const aiScene *scene,
                                        const std::string &path,
                                        bool generateLODs,
                                        const std::vector<float> &lodLevels);

  // 处理Assimp的Mesh数据
  static MeshData ProcessMesh(const aiMesh *aiMesh, VertexLayout layout);

  // 处理顶点布局描述（供Renderer模块使用）
  static VertexLayout GenerateVertexLayout(const aiMesh *aiMesh);

  // 使用meshoptimizer简化网格
  static MeshData SimplifyMesh(const MeshData &originalMesh,
                               float targetRatio,
                               VertexLayout layout);

  // 计算模型的包围盒
  static void CalculateBoundingBox(const std::vector<MeshDataLODChain> &subMeshes,
                                   glm::vec3 &outMin,
                                   glm::vec3 &outMax);

  // 创建合并的模型资源数据（以及合并的同时创建MeshSectionLODChain）
  static std::shared_ptr<ModelSourceData> CreateModelSourceData(
      std::shared_ptr<ModelAsset> model, const std::vector<MeshDataLODChain> &subMeshData);

  // 配置Assimp导入器标志（格式特化）
  static unsigned int GetAssimpImportFlags(const std::string &extension, bool flipUVs);

  // 通过路径查找已缓存的模型
  static ModelAssetID FindModelByPath(ModelCache &cache, const std::string &path);
};
};  // namespace mite

#endif
