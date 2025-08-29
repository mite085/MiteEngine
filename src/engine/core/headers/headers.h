#ifndef MITE_INCLUDED_HEADERS
#define MITE_INCLUDED_HEADERS

// C++标准库
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <execution>
#include <filesystem>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <sstream>
#include <typeindex>
#include <unordered_set>
#include <unordered_map>
#include <variant>
#include <vector>

// C++17 标准库
#include <any>
#include <optional>

// 核心函数模块
#include "core/core_functions.h"

// 数学库（使用gtx相关函数，需要启用EXPERIMENTAL）
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>

// 文件系统
#include "filesystem/filesystem.h"

// 日志系统
#include "logger/logger.h"

// UUID系统
#include "uuid/mite_uuid.h"

// 时间系统
#include "time/time.h"

// 事件系统
#include "event/subscription_group.h"
#include "event/callback_adapter.h"

#endif