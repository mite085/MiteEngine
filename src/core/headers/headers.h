#ifndef MITE_INCLUDED_HEADERS
#define MITE_INCLUDED_HEADERS

// C++标准库
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <string>
#include <functional>
#include <algorithm>
#include <mutex>
#include <sstream>
#include <array>
#include <typeindex>
#include <cassert>
#include <atomic>
#include <filesystem>

// C++17 标准库
#include <any>
#include <optional>

// 核心函数模块
#include "core/core_functions.h"

// 数学库
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

// 日志系统
#include "logger/logger.h"

// 时间系统
#include "time/time.h"

// 事件系统
#include "event/event_types.h"
#include "event/event.h"
#include "event/dispatcher.h"

#endif