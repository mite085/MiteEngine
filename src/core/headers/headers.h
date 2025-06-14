#ifndef MITE_INCLUDED_HEADERS
#define MITE_INCLUDED_HEADERS

// C++标准库
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

// C++17 标准库
#include <any>
#include <optional>

// 核心函数模块
#include "core/core_functions.h"

// 数学库（使用gtx相关函数，需要启用EXPERIMENTAL）
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

// 日志系统
#include "logger/logger.h"

// 时间系统
#include "time/time.h"

// 事件系统
#include "event/event_types.h"
#include "event/event.h"
#include "event/dispatcher.h"

#endif