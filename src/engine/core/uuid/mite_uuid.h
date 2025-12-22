#ifndef MITE_CORE_UUID_GENERATER
#define MITE_CORE_UUID_GENERATER

#include <uuid.h>

namespace mite {

using UUID = uuids::uuid;

/**
 * @brief UUID生成器
 *
 * 静态，单例，外部仅需调用Generate()方法，返回uuids::uuid
 * 优化点：
 * 1. 使用线程局部存储的随机数生成器避免重复初始化
 * 2. 优化字符串处理减少临时对象
 * 3. 保证线程安全
 */
class UUIDGenerator {
 private:
  // 线程局部存储的随机数生成器，避免每次调用都初始化
  static inline std::mt19937 &GetThreadLocalGenerator()
  {
    thread_local std::random_device rd;
    thread_local std::mt19937 generator([&] {
      auto seed_data = std::array<int, std::mt19937::state_size>{};
      std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
      std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
      return std::mt19937(seq);
    }());
    return generator;
  }

 public:
  /**
   * @brief 生成完全随机的UUID
   * @return uuids::uuid 生成的UUID
   * 复杂度: O(1) 平均时间复杂度
   */
  static uuids::uuid Generate()
  {
    // C6001 使用未初始化的内存“bytes”
    // 这个警告是误报，bytes数组会被完全初始化
    thread_local uuids::uuid_random_generator gen{GetThreadLocalGenerator()};
    return gen();
  }

  /**
   * @brief 根据索引生成伪随机的UUID
   * @param index 生成UUID的索引值
   * @return uuids::uuid 生成的UUID
   */
  static uuids::uuid Generate(size_t index)
  {
    constexpr size_t fixed_seed = 0x123456789ABCDEF0;
    std::mt19937 generator(unsigned int(fixed_seed + index));   // 每次重新构造
    uuids::uuid_random_generator gen(generator);  // 局部变量，不复用
    return gen();                                 // 确保每次调用都从初始状态开始
  }

  /**
   * @brief 根据字符串索引生成伪随机的UUID
   * @param str 用于生成UUID的字符串
   * @return uuids::uuid 生成的UUID
   * 复杂度: O(n) n为字符串长度
   * 优化: 直接计算哈希，避免创建临时string对象
   */
  static uuids::uuid Generate(const char *str)
  {
    // 直接计算C字符串的哈希，避免创建临时string对象
    size_t hash = 5381;
    int c;
    while (*str) {
      c = *str++;
      hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }
    return Generate(hash);
  }

  // UUID字符串转换的本地辅助函数
  static std::string UUIDToString(const UUID &id)
  {
    return uuids::to_string(id);
  }

  static UUID UUIDFromString(const std::string &id)
  {
    auto optionalUUID = uuids::uuid::from_string(id);
    if (optionalUUID.has_value())
      return optionalUUID.value();
    else
      return {};
  }
};

};  // namespace mite

#endif
