#ifndef MITE_CORE_FILESYSTEM
#define MITE_CORE_FILESYSTEM

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "logger/logger.h"
// 通过CmakeLists的target_include_directories(PRIVATE
// ${CMAKE_BINARY_DIR}/src/core/filesystem)
// 检索到build/src/core/filesystem/filesystem_config.h文件
#include "filesystem_config.h"

namespace mite {
/**
 * @brief 文件管理函数
 *
 */
class FileSystem {
 public:
  // 初始化文件系统
  static void Initialize();

  // 获取资源完整路径
  static std::filesystem::path GetAssetPath(const std::string &relativePath);

  // 获取资源根目录（exe同级目录下的assets）
  static std::filesystem::path GetAssetsRoot();

  // 检查文件是否存在
  static bool Exists(const std::filesystem::path &path);

  // 读取文件内容
  static std::string ReadFileToString(const std::filesystem::path &path);

  // 写入文件内容
  static bool WriteStringToFile(const std::filesystem::path &path,
                                const std::string &content);

 private:
  // 获取可执行文件路径
  static std::filesystem::path GetExecutablePath();

  static std::filesystem::path s_ExecutablePath;
  static bool s_Initialized;
  static Logger s_Logger;
};
};  // namespace mite

#endif
