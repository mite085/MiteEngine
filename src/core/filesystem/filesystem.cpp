#include "filesystem.h"

#ifdef FS_PLATFORM_WINDOWS
#  include <windows.h>
#elif defined(FS_PLATFORM_LINUX) || defined(FS_PLATFORM_MACOS)
#  include <limits.h>
#  include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace mite {
// 静态成员初始化
fs::path FileSystem::s_executablePath;
std::vector<fs::path> FileSystem::s_assetRoots;
bool FileSystem::s_initialized = false;

void FileSystem::Init(int argc, char **argv)
{
  if (s_initialized)
    return;

  s_executablePath = GetExecutablePath();
  InitializeAssetRoots();
  s_initialized = true;
}

fs::path FileSystem::GetAssetPath(const std::string &relativePath)
{
  if (!s_initialized) {
    throw std::runtime_error("FileSystem not initialized. Call FileSystem::Initialize() first.");
  }

  for (const auto &root : s_assetRoots) {
    fs::path fullPath = root / relativePath;
    if (Exists(fullPath)) {
      return fullPath;
    }
  }

  std::string errorMsg = "Asset not found: " + relativePath + "\nSearched in:";
  for (const auto &root : s_assetRoots) {
    errorMsg += "\n- " + root.string();
  }
  throw std::runtime_error(errorMsg);
}

const std::vector<fs::path> &FileSystem::GetAssetRoots()
{
  if (!s_initialized) {
    throw std::runtime_error("FileSystem not initialized. Call FileSystem::Initialize() first.");
  }
  return s_assetRoots;
}

bool FileSystem::Exists(const fs::path &path)
{
  std::error_code ec;
  bool exists = fs::exists(path, ec);
  return !ec && exists;
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

  // 读取内容
  std::string content;
  content.resize(static_cast<size_t>(size));
  file.read(&content[0], size);

  return content;
}

bool FileSystem::WriteStringToFile(const fs::path &path, const std::string &content)
{
  std::error_code ec;
  fs::create_directories(path.parent_path(), ec);
  if (ec)
    return false;

  std::ofstream file(path, std::ios::binary);
  if (!file.is_open())
    return false;

  file.write(content.data(), content.size());
  return !file.fail();
}

fs::path FileSystem::GetExecutablePath()
{
#ifdef _WIN32
  wchar_t path[MAX_PATH] = {0};
  GetModuleFileNameW(nullptr, path, MAX_PATH);
  return fs::path(path).parent_path();
#else
  char path[PATH_MAX] = {0};
#  if defined(__linux__)
  ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
  if (count != -1) {
    return fs::path(path).parent_path();
  }
#  elif defined(__APPLE__)
  uint32_t size = sizeof(path);
  if (_NSGetExecutablePath(path, &size) == 0) {
    char realPath[PATH_MAX] = {0};
                if (realpath(path, realPath) {
      return fs::path(realPath).parent_path();
                }
  }
#  endif
  return fs::current_path();
#endif
}

void FileSystem::InitializeAssetRoots()
{
  s_assetRoots.clear();

  // 1. 可执行文件同级目录
  s_assetRoots.push_back(s_executablePath / "assets");

  // 2. 可执行文件父目录
  s_assetRoots.push_back(s_executablePath.parent_path() / "assets");

  // 3. 构建目录 (来自CMake配置)
  s_assetRoots.push_back(fs::path(ASSETS_BUILD_DIR));

  // 4. 源代码目录
  s_assetRoots.push_back(fs::path(ASSETS_SOURCE_DIR));

  // 5. 安装目录
  s_assetRoots.push_back(fs::path(ASSETS_INSTALL_DIR));

  // 6. 环境变量目录
  if (const char *envPath = std::getenv("ASSETS_PATH")) {
    s_assetRoots.push_back(fs::path(envPath));
  }

  // 7. 当前工作目录
  s_assetRoots.push_back(fs::current_path() / "assets");
}
};
