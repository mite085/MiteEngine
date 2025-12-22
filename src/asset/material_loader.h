#ifndef MITE_ASSET_MATERIAL_LOADER
#define MITE_ASSET_MATERIAL_LOADER

#include "asset_cache.h"
#include "basic_event/asset_event.h"

struct aiMaterial;
struct aiScene;

namespace mite {
/**
 * 材质加载器 - 专门处理GLTF PBR材质导入
 * 职责：
 * 1. 从Assimp场景中提取材质信息并转换为MaterialAsset
 * 2. 主动创建纹理资产并建立引用关系
 * 3. 生成标准的材质元数据供MaterialSystem使用
 */
class MaterialLoader {
 public:
  /**
   * 从GLTF文件加载所有材质到缓存
   * @param materialCache 材质缓存引用
   * @param textureCache 纹理缓存引用（用于纹理依赖）
   * @param scene Assimp场景对象
   * @param modelPath 模型文件路径
   * @return 加载的材质AssetID列表
   */
  static std::vector<MaterialAssetID> LoadMaterialsFromGLTF(
      MaterialCache &materialCache, TextureCache &textureCache,
      const aiScene *scene, const std::string &modelPath);

 private:
  /**
   * 处理单个Assimp材质到缓存
   */
  static MaterialAssetID ProcessGLTFMaterial(MaterialCache &materialCache,
                                             TextureCache &textureCache,
                                             aiMaterial *aiMat,
                                             uint32_t materialIndex,
                                             const std::string &modelPath,
                                             const aiScene *scene);

  /**
   * 提取GLTF PBR材质参数
   */
  static void ExtractPBRParameters(aiMaterial *aiMat,
                                   MaterialMetadata &metadata);

  /**
   * 提取并创建材质纹理引用
   */
  static void ExtractAndCreateTextureReferences(TextureCache &textureCache,
                                                aiMaterial *aiMat,
                                                MaterialMetadata &metadata,
                                                const std::string &modelPath,
                                                const aiScene *scene);

  /**
   * 根据纹理路径创建或获取纹理资产ID
   */
  static TextureAssetID CreateOrGetTextureAssetID(
      TextureCache &textureCache, const std::string &texturePath,
      const std::string &modelPath, const aiScene *scene);

  /**
   * 生成材质唯一名称
   */
  static std::string GenerateMaterialName(aiMaterial *aiMat, uint32_t index,
                                          const std::string &modelPath);

  /**
   * 解析纹理完整路径（处理相对路径）
   */
  static std::string ResolveTexturePath(const std::string &texturePath,
                                        const std::string &modelPath);
};
}  // namespace mite

#endif
