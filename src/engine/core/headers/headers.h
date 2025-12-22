#ifndef MITE_INCLUDED_HEADERS
#define MITE_INCLUDED_HEADERS

// C++标准库
#include <stdint.h>

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
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <regex>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

// C++17 标准库
#include <any>
#include <optional>

// GLAD+GLFW
#include <glad.h>
// GLFW必须在GLAD加载库之后
#include <GLFW/glfw3.h>

// 数学库（使用gtx相关函数，需要启用EXPERIMENTAL）
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>

// 日志系统
#include "logger/logger.h"

// UUID系统
#include "uuid/mite_uuid.h"

#endif