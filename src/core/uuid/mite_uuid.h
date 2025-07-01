#ifndef MITE_CORE_UUID_GENERATER
#define MITE_CORE_UUID_GENERATER

#include <uuid.h>

namespace mite {
/**
 * @brief UUID生成器
 * 
 * 静态，单例，外部仅需调用Generate()方法，返回uuids::uuid
 */
class UUIDGenerator {
 public:
  // 生成完全随机的UUID
  static uuids::uuid Generate()
  {
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size>{};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 generator(seq);
    uuids::uuid_random_generator gen{generator};

    return gen();
  }
  // 根据索引生成伪随机的UUID
  static uuids::uuid Generate(size_t index)
  {
    // 使用固定种子加上索引作为随机源
    constexpr size_t fixed_seed = 0x123456789ABCDEF0;
    std::mt19937 generator(unsigned int(fixed_seed + index));
    uuids::uuid_random_generator gen{generator};

    return gen();
  }
  // 根据字符串索引生成伪随机的UUID
  static uuids::uuid Generate(const char *str)
  {
    // 使用字符串哈希作为种子
    std::hash<std::string> hasher;
    size_t hash = hasher(std::string(str));
    return Generate(hash);
  }
};

};  // namespace mite

#endif
