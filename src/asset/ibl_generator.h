#ifndef MITE_IBL_GENERATOR_H
#define MITE_IBL_GENERATOR_H

#include "GltfIblSampler.h"
#include "basic_type/texture_type.h"
#include "headers/headers.h"

namespace mite {

// IBL配置
struct IBLConfig {
  int qualityLevel = 1;  // 0=Low, 1=Medium, 2=High
  std::string cacheDirectory = "cache/ibl";
  bool autoGenerateIBL = true;
};

/**
 * IBL纹理生成器 - 封装GltfIblSampler功能
 * 现在作为TextureLoader的内部工具类
 */
class IBLGenerator {
 public:
  struct GenerateOptions {
    uint32_t irradianceSize = 64;
    uint32_t prefilterSize = 512;
    uint32_t prefilterLevels = 8;
    uint32_t sampleCount = 1024;
    IBLLib::Distribution distribution = IBLLib::Distribution::GGX;
    IBLLib::OutputFormat format = IBLLib::OutputFormat::R16G16B16A16_SFLOAT;
  };

  // 静态工具函数，无需单例模式
  static bool GenerateIBLTextures(const std::string &hdrInputPath,
                                  const std::string &outputDir,
                                  const GenerateOptions &options = GenerateOptions());

  static GenerateOptions GetOptionsForQuality(int qualityLevel);
  static std::string GenerateEnvironmentId(const std::string &hdrPath);

 private:
  IBLGenerator() = delete;
  ~IBLGenerator() = delete;
};

}  // namespace mite

#endif
