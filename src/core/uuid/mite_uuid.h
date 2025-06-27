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
};

};  // namespace mite

#endif
