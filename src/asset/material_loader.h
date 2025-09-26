#ifndef MITE_ASSET_MATERIAL_LOADER
#define MITE_ASSET_MATERIAL_LOADER

#include "basic_type/asset_type.h"
#include "headers/headers.h"

struct aiMaterial;
struct aiScene;

namespace mite {

/**
 * 材质加载器 - 专门处理GLTF PBR材质导入
 * 职责：
 * 1. 从Assimp场景中提取材质信息并转换为MaterialAsset
 * 2. 处理材质参数和纹理引用
 * 3. 生成标准的材质元数据供MaterialSystem使用
 */
class MaterialLoader {
 public:
  /**
   * 从GLTF文件加载所有材质
   * @param scene Assimp场景对象
   * @param modelPath 模型文件路径（用于纹理路径解析）
   * @param loadedTextures 已加载的纹理资产列表（用于关联引用）
   * @return 材质资产列表，失败返回空列表
   */
  static std::vector<std::shared_ptr<MaterialAsset>> LoadMaterialsFromGLTF(
      const aiScene *scene,
      const std::string &modelPath,
      const std::vector<std::shared_ptr<TextureAsset>> &loadedTextures);

  /**
   * 创建内置测试材质（纯色材质）
   * @param name 材质名称
   * @param color 基础颜色
   * @return 材质资产指针
   */
  static std::shared_ptr<MaterialAsset> CreateBuiltinMaterial(
      const std::string &name = "PureColor", const glm::vec3 &color = glm::vec3(1.0f));

 private:
  /**
   * 处理单个Assimp材质
   */
  static std::shared_ptr<MaterialAsset> ProcessGLTFMaterial(
      aiMaterial *aiMat,
      uint32_t materialIndex,
      const std::string &modelPath,
      const std::vector<std::shared_ptr<TextureAsset>> &loadedTextures);

  /**
   * 提取GLTF PBR材质参数
   */
  static void ExtractPBRParameters(aiMaterial *aiMat, MaterialMetadata &metadata);

  /**
   * 提取材质纹理引用
   */
  static void ExtractTextureReferences(
      aiMaterial *aiMat,
      MaterialMetadata &metadata,
      const std::string &modelPath,
      const std::vector<std::shared_ptr<TextureAsset>> &loadedTextures);

  /**
   * 根据纹理路径查找对应的TextureAssetID
   */
  static TextureAssetID FindTextureAssetID(
      const std::string &texturePath,
      const std::vector<std::shared_ptr<TextureAsset>> &loadedTextures);

  /**
   * 提取纹理变换参数（缩放/偏移）
   */
  static void ExtractTextureTransform(aiMaterial *aiMat,
                                      aiTextureType textureType,
                                      unsigned int textureIndex,
                                      glm::vec2 &scale,
                                      glm::vec2 &offset);

  /**
   * 生成材质唯一名称
   */
  static std::string GenerateMaterialName(aiMaterial *aiMat,
                                          uint32_t index,
                                          const std::string &modelPath);
};

}  // namespace mite

#endif
