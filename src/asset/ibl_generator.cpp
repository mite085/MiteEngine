#include "ibl_generator.h"
#include <filesystem>

namespace mite {

bool IBLGenerator::GenerateIBLTextures(const std::string &hdrInputPath,
                                       const std::string &outputDir,
                                       const GenerateOptions &options)
{
  if (!std::filesystem::exists(hdrInputPath)) {
    LOG_ERROR("HDR input file does not exist: " + hdrInputPath);
    return false;
  }

  std::filesystem::create_directories(outputDir);

  std::string cubeMapPath = outputDir + "/environment.ktx2";
  std::string brdfLUTPath = outputDir + "/brdf_lut.ktx2";

  LOG_INFO("Generating IBL textures:");
  LOG_INFO("  Input: " + hdrInputPath);
  LOG_INFO("  CubeMap: " + cubeMapPath);
  LOG_INFO("  BRDF LUT: " + brdfLUTPath);

  IBLLib::Result result = IBLLib::sample(hdrInputPath.c_str(),
                                         cubeMapPath.c_str(),
                                         brdfLUTPath.c_str(),
                                         options.distribution,
                                         options.prefilterSize,
                                         options.prefilterLevels,
                                         options.sampleCount,
                                         options.format,
                                         0.0f,
                                         false);

  if (result != IBLLib::Result::Success) {
    LOG_ERROR("Failed to generate IBL textures");
    return false;
  }

  LOG_INFO("Successfully generated IBL textures");
  return true;
}

IBLGenerator::GenerateOptions IBLGenerator::GetOptionsForQuality(int qualityLevel)
{
  GenerateOptions options;

  switch (qualityLevel) {
    case 0:  // Low
      options.irradianceSize = 32;
      options.prefilterSize = 256;
      options.prefilterLevels = 6;
      options.sampleCount = 512;
      options.format = IBLLib::OutputFormat::R8G8B8A8_UNORM;
      break;

    case 1:  // Medium
      options.irradianceSize = 64;
      options.prefilterSize = 512;
      options.prefilterLevels = 8;
      options.sampleCount = 1024;
      options.format = IBLLib::OutputFormat::R16G16B16A16_SFLOAT;
      break;

    case 2:  // High
      options.irradianceSize = 128;
      options.prefilterSize = 1024;
      options.prefilterLevels = 10;
      options.sampleCount = 2048;
      options.format = IBLLib::OutputFormat::R32G32B32A32_SFLOAT;
      break;

    default:
      options = GetOptionsForQuality(1);
      break;
  }

  return options;
}

std::string IBLGenerator::GenerateEnvironmentId(const std::string &hdrPath)
{
  std::filesystem::path path(hdrPath);
  return path.stem().string() + "_" + std::to_string(std::filesystem::file_size(hdrPath));
}

}  // namespace mite
