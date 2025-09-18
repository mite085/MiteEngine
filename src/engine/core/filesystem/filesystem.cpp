#include "filesystem.h"


#ifdef _WIN32
#  include <windows.h>
#else
#  include <limits.h>
#  include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace mite {
// 静态成员初始化
fs::path FileSystem::s_ExecutablePath;
bool FileSystem::s_Initialized = false;
Logger FileSystem::s_Logger = nullptr;
void FileSystem::Initialize(int argc, char **argv)
{
  if (s_Initialized)
    return;
  s_ExecutablePath = GetExecutablePath();
  s_Initialized = true;
  s_Logger = LoggerSystem::CreateModuleLogger("Mite File System");
  s_Logger->info("FileSystem initialized. Executable path: {}", s_ExecutablePath.string());
  s_Logger->info("Assets root: {}", GetAssetsRoot().string());
}
fs::path FileSystem::GetAssetPath(const std::string &relativePath)
{
  if (!s_Initialized) {
    throw std::runtime_error("FileSystem not initialized. Call FileSystem::Init() first.");
  }
  fs::path assetsRoot = GetAssetsRoot();
  fs::path fullPath = assetsRoot / relativePath;
  if (!Exists(fullPath)) {
    throw std::runtime_error("Asset not found: " + relativePath + "\nFull path: " +
                             fullPath.string() + "\nAssets root: " + assetsRoot.string());
  }
  return fullPath;
}
fs::path FileSystem::GetAssetsRoot()
{
  if (!s_Initialized) {
    throw std::runtime_error("FileSystem not initialized. Call FileSystem::Init() first.");
  }

  // 直接从exe同级目录下的assets文件夹
  return s_ExecutablePath / ASSETS_DIR;
}
bool FileSystem::Exists(const fs::path &path)
{
  std::error_code ec;
  bool exists = fs::exists(path, ec);

  if (ec) {
    s_Logger->warn(
        "Error checking file existence: {}, error: {}", path.string(), ec.message());
    return false;
  }

  return exists;
}
std::string FileSystem::ReadFileToString(const fs::path &path)
{
  if (!Exists(path)) {
    throw std::runtime_error("File not found: " + path.string());
  }
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + path.string());
  }
  // 获取文件大小
  file.seekg(0, std::ios::end);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);
  if (size == 0) {
    return "";
  }
  // 读取内容
  std::string content;
  content.resize(static_cast<size_t>(size));
  file.read(&content[0], size);
  if (file.fail()) {
    throw std::runtime_error("Failed to read file: " + path.string());
  }
  return content;
}
bool FileSystem::WriteStringToFile(const fs::path &path, const std::string &content)
{
  try {
    // 创建父目录（如果不存在）
    fs::create_directories(path.parent_path());

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
      s_Logger->error("Failed to open file for writing: {}", path.string());
      return false;
    }
    file.write(content.data(), content.size());
    return !file.fail();
  }
  catch (const std::exception &e) {
    s_Logger->error("Error writing to file {}: {}", path.string(), e.what());
    return false;
  }
}
fs::path FileSystem::GetExecutablePath()
{
#ifdef _WIN32
  wchar_t path[MAX_PATH] = {0};
  DWORD result = GetModuleFileNameW(nullptr, path, MAX_PATH);
  if (result == 0 || result == MAX_PATH) {
    s_Logger->warn("Failed to get executable path, using current directory");
    return fs::current_path();
  }
  return fs::path(path).parent_path();
#else
  char path[PATH_MAX] = {0};

#  if defined(__linux__)
  ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
  if (count == -1 || count >= PATH_MAX) {
    s_Logger->warn("Failed to get executable path, using current directory");
    return fs::current_path();
  }
  path[count] = '\0';
#  elif defined(__APPLE__)
  uint32_t size = sizeof(path);
  if (_NSGetExecutablePath(path, &size) != 0) {
    s_Logger->warn("Failed to get executable path, using current directory");
    return fs::current_path();
  }

  char realPath[PATH_MAX] = {0};
  if (realpath(path, realPath) == nullptr) {
    s_Logger->warn("Failed to resolve executable path, using: {}", path);
    return fs::path(path).parent_path();
  }
  return fs::path(realPath).parent_path();
#  endif
  return fs::path(path).parent_path();
#endif
}
};
