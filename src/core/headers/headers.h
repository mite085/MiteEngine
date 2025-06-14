#ifndef MITE_INCLUDED_HEADERS
#define MITE_INCLUDED_HEADERS

// C++��׼��
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

// C++17 ��׼��
#include <any>
#include <optional>

// ���ĺ���ģ��
#include "core/core_functions.h"

// ��ѧ�⣨ʹ��gtx��غ�������Ҫ����EXPERIMENTAL��
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

// ��־ϵͳ
#include "logger/logger.h"

// ʱ��ϵͳ
#include "time/time.h"

// �¼�ϵͳ
#include "event/event_types.h"
#include "event/event.h"
#include "event/dispatcher.h"

#endif