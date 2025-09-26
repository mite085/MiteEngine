#ifndef MITE_ASSET_TEXTURE_LOADER
#define MITE_ASSET_TEXTURE_LOADER

#include "basic_type/asset_type.h"
#include "headers/headers.h"

class aiTexture;

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
   * 加载外部纹理文件
   * @param path 纹理文件路径
   * @param desiredChannels 期望通道数（0=保持原样，3=RGB，4=RGBA）
   * @param flipVertical 是否垂直翻转（适配OpenGL坐标系）
   * @return 纹理资产指针，失败返回nullptr
   */
  static std::shared_ptr<TextureAsset> LoadTexture(const std::string &path,
                                                   int desiredChannels = 4,
                                                   bool flipVertical = true);
  /**
   * 专门处理Assimp嵌入式纹理
   * @param embeddedId 嵌入式纹理标识（如"*0"）
   * @param modelPath 模型文件路径（用于生成唯一路径标识）
   * @param aiTexture Assimp纹理对象
   * @param desiredChannels 期望通道数
   * @param flipVertical 是否垂直翻转
   * @return 纹理资产指针，失败返回nullptr
   */
  static std::shared_ptr<TextureAsset> LoadEmbeddedTexture(const std::string &embeddedId,
                                                           const std::string &modelPath,
                                                           const aiTexture *aiTexture,
                                                           int desiredChannels = 4,
                                                           bool flipVertical = true);
  /**
   * 检查路径是否为嵌入式纹理标识（以'*'开头）
   */
  static bool IsEmbeddedTexturePath(const std::string &path);

  /**
   * 根据通道数确定纹理格式
   */
  static TextureFormat DetermineTextureFormat(int channels);

  /**
   * 根据文件扩展名推断纹理目标类型
   */
  static TextureTarget DetermineTextureTarget(const std::string &path);

 private:
  /**
   * 核心加载实现 - 处理像素数据加载和资产构建
   */
  static std::shared_ptr<TextureAsset> LoadTextureInternal(
      const std::string &path,
      const std::vector<uint8_t> &embeddedData,
      int desiredChannels,
      bool flipVertical);
  /**
   * 从Assimp纹理对象提取嵌入式数据
   */
  static std::vector<uint8_t> ExtractEmbeddedData(const aiTexture *aiTexture);
  /**
   * 设置默认采样参数（可后续通过材质系统覆盖）
   */
  static void SetupDefaultSamplingParams(TextureMetadata &metadata);
};
};  // namespace mite

#endif
