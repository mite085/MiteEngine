#ifndef MITE_ASSET_TEXTURE_LOADER
#define MITE_ASSET_TEXTURE_LOADER

#include "basic_event/asset_event.h"
#include "ibl_generator.h"
#include "ktx2_loader.h"
#include "asset_cache.h"

struct aiTexture;

namespace mite {
/**
 * 纹理加载器 - 纯数据解析，不涉及GPU资源创建
 * 职责：
 * 1. 解析外部纹理文件（PNG/JPG/BMP等）为TextureAsset
 * 2. 处理Assimp嵌入式纹理（GLTF内嵌纹理）
 * 3. 生成标准的TextureSourceData供Renderer使用
 */
class TextureLoader {
 public:
  /**
   * @brief 加载外部纹理文件到缓存
   * @param cache 纹理缓存引用
   * @param path 纹理文件路径
   * @param desiredChannels 期望通道数（0=保持原样，3=RGB，4=RGBA）
   * @param flipVertical 是否垂直翻转（适配OpenGL坐标系）
   * @return 纹理资产ID，失败返回无效ID
   */
  static TextureAssetID LoadTexture(TextureCache &cache,
                                    const std::string &path,
                                    int desiredChannels = 4,
                                    bool flipVertical = true);
  /**
   * @brief 专门处理Assimp嵌入式纹理到缓存
   * @param cache 纹理缓存引用
   * @param embeddedId 嵌入式纹理标识（如"*0"）
   * @param modelPath 模型文件路径（用于生成唯一路径标识）
   * @param aiTexture Assimp纹理对象
   * @param desiredChannels 期望通道数
   * @param flipVertical 是否垂直翻转
   * @return 纹理资产ID，失败返回无效ID
   */
  static TextureAssetID LoadEmbeddedTexture(TextureCache &cache,
                                            const std::string &embeddedId,
                                            const std::string &modelPath,
                                            const aiTexture *aiTexture,
                                            int desiredChannels = 4,
                                            bool flipVertical = true);
  /**
   * @brief 加载HDR纹理到缓存
   * @param cache 纹理缓存引用
   * @param path 纹理文件路径
   * @param flipVertical 是否垂直翻转（适配OpenGL坐标系）
   * @return  纹理资产ID，失败返回无效ID
   */
  static TextureAssetID LoadHDRTexture(TextureCache &cache,
                                       const std::string &path,
                                       bool flipVertical = true);
  
  
  /**
   * @brief 加载环境纹理到缓存，并生成对应IBL纹理
   * @param cache  纹理缓存引用
   * @param hdrPath 纹理文件路径
   * @param flipVertical 是否垂直翻转（适配OpenGL坐标系）
   * @return 纹理资产ID（仅原始纹理资产）
   */
  static TextureAssetID LoadEnvironmentTexture(TextureCache &cache,
                                               const std::string &hdrPath,
                                               bool flipVertical = true);
  // IBL纹理配置
  static void SetIBLConfig(const IBLConfig &config);
  static const IBLConfig &GetIBLConfig();
  static bool IsKTX2File(const std::string &path);
  static bool IsHDRFile(const std::string &path);

  /**
   * 检查路径是否为嵌入式纹理标识（以'*'开头）
   */
  static bool IsEmbeddedTexturePath(const std::string &path);

 private:
  /**
   * @brief 核心加载实现 - 处理像素数据加载和资产构建
   * @note 普通2D LDR纹理 / HDR纹理 / 环境纹理分离逻辑
   */
  static TextureAssetID LoadTextureInternal(TextureCache &cache,
                                            const std::string &path,
                                            const std::vector<uint8_t> &embeddedData,
                                            int desiredChannels,
                                            bool flipVertical);
  static TextureAssetID LoadHDRTextureInternal(TextureCache &cache,
                                               const std::string &path,
                                               bool flipVertical);
  static TextureAssetID LoadEnvironmentTextureInternal(TextureCache &cache,
                                                       const std::string &path,
                                                       bool flipVertical);
  /**
   * 从Assimp纹理对象提取嵌入式数据
   */
  static std::vector<uint8_t> ExtractEmbeddedData(const aiTexture *aiTexture);
  /**
   * 设置默认采样参数（可后续通过材质系统覆盖）
   */
  static void SetupDefaultSamplingParams(TextureMetadata &metadata);
  /**
   * 通过路径查找已缓存的纹理
   */
  static TextureAssetID FindTextureByPath(TextureCache &cache, const std::string &path);
  /**
   * 根据通道数确定纹理格式(LDR / HDR)
   */
  static TextureFormat DetermineLDRTextureFormat(int channels);
  static TextureFormat DetermineHDRTextureFormat(int channels);
  /**
   * HDR纹理采样参数
   */
  static void SetupHDRSamplingParams(TextureMetadata &metadata);
  /**
   * 像素大小计算
   */
  static size_t GetPixelSize(TextureFormat format);
  /**
   * IBL生成相关
   */
  static bool GenerateAndLoadIBLTextures(TextureCache &cache,
                                         const std::string &hdrPath,
                                         const std::string &environmentId);

  static std::string GetIBLCachePath(const std::string &environmentId);
  static bool ShouldGenerateIBL(const std::string &environmentId);

  static IBLConfig s_IBLConfig;
};
};  // namespace mite

#endif
