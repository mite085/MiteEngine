#ifndef MITE_INCLUDED_HEADERS
#define MITE_INCLUDED_HEADERS

// C++标准库
#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <cassert>
#include <condition_variable>
#include <cstdint>
#include <execution>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <regex>
#include <set>
#include <shared_mutex>
#include <stack>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <sstream>
#include <thread>
#include <typeindex>
#include <unordered_set>
#include <unordered_map>
#include <variant>
#include <vector>

// C++17 标准库
#include <any>
#include <optional>

// GLAD+GLFW
#include <glad.h>
#include <glfw/glfw3.h>  // 必须在GLAD加载库之后

// 数学库（使用gtx相关函数，需要启用EXPERIMENTAL）
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

// 文件系统
#include "filesystem/filesystem.h"

// 日志系统
#include "logger/logger.h"

// UUID系统
#include "uuid/mite_uuid.h"

// 时间系统
#include "time/time.h"

// 线程池
#include "thread/thread_pool_manager.h"
#include "thread/parallel_utils.h"

#endif