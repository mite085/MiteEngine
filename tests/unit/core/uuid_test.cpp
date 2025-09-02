#include "uuid/mite_uuid.h"
#include "gtest/gtest.h"
#include <atomic>
#include <thread>
#include <unordered_set>
#include <vector>
// 测试夹具类，用于设置测试环境和共享资源
class UUIDTest : public ::testing::Test {
 protected:
  void SetUp() override
  {
    // 每个测试用例执行前的初始化代码
  }
  void TearDown() override
  {
    // 每个测试用例执行后的清理代码
  }
  // 辅助函数：验证UUID是否有效
  static bool IsValidUUID(const uuids::uuid &id)
  {
    return !id.is_nil() && id.version() != uuids::uuid_version::none;
  }
  // 辅助函数：验证两个UUID是否不同
  static bool AreUUIDsDifferent(const uuids::uuid &id1, const uuids::uuid &id2)
  {
    return id1 != id2;
  }
};

// 测试用例1：基本功能测试 - 生成有效的UUID
TEST_F(UUIDTest, GenerateValidUUID)
{
  // 测试生成随机UUID
  auto uuid = mite::UUIDGenerator::Generate();

  // 验证UUID不为空且版本有效
  EXPECT_FALSE(uuid.is_nil());
  EXPECT_NE(uuid.version(), uuids::uuid_version::none);

  // 验证UUID的版本应该是随机版本（版本4）
  EXPECT_EQ(uuid.version(), uuids::uuid_version::random_number_based);
}

// 测试用例2：唯一性测试 - 确保生成的UUID都是唯一的
TEST_F(UUIDTest, GenerateUniqueUUIDs)
{
  constexpr int NUM_UUIDS = 1000;
  std::unordered_set<std::string> uuid_set;

  // 生成大量UUID并检查唯一性
  for (int i = 0; i < NUM_UUIDS; ++i) {
    auto uuid = mite::UUIDGenerator::Generate();
    std::string uuid_str = uuids::to_string(uuid);

    // 验证UUID不在集合中（唯一性）
    EXPECT_EQ(uuid_set.find(uuid_str), uuid_set.end());
    uuid_set.insert(uuid_str);

    // 验证UUID有效性
    EXPECT_TRUE(IsValidUUID(uuid));
  }

  // 验证生成了正确数量的唯一UUID
  EXPECT_EQ(uuid_set.size(), NUM_UUIDS);
}

// 测试用例3：基于索引的确定性生成测试
TEST_F(UUIDTest, GenerateFromIndexDeterministic)
{
  const size_t test_index = 42;

  // 使用相同索引生成两次UUID
  auto uuid1 = mite::UUIDGenerator::Generate(test_index);
  auto uuid2 = mite::UUIDGenerator::Generate(test_index);

  // 验证两次生成的结果相同（确定性）
  EXPECT_EQ(uuid1, uuid2);
  EXPECT_TRUE(IsValidUUID(uuid1));
  EXPECT_TRUE(IsValidUUID(uuid2));
}

// 测试用例4：不同索引生成不同UUID
TEST_F(UUIDTest, GenerateFromDifferentIndexes)
{
  // 使用不同索引生成UUID
  auto uuid1 = mite::UUIDGenerator::Generate(1);
  auto uuid2 = mite::UUIDGenerator::Generate(2);
  auto uuid3 = mite::UUIDGenerator::Generate(3);

  // 验证所有UUID都是有效且不同的
  EXPECT_TRUE(IsValidUUID(uuid1));
  EXPECT_TRUE(IsValidUUID(uuid2));
  EXPECT_TRUE(IsValidUUID(uuid3));
  EXPECT_TRUE(AreUUIDsDifferent(uuid1, uuid2));
  EXPECT_TRUE(AreUUIDsDifferent(uuid1, uuid3));
  EXPECT_TRUE(AreUUIDsDifferent(uuid2, uuid3));
}

// 测试用例5：基于字符串的确定性生成测试
TEST_F(UUIDTest, GenerateFromStringDeterministic)
{
  const char *test_string = "test_string_123";

  // 使用相同字符串生成两次UUID
  auto uuid1 = mite::UUIDGenerator::Generate(test_string);
  auto uuid2 = mite::UUIDGenerator::Generate(test_string);

  // 验证两次生成的结果相同（确定性）
  EXPECT_EQ(uuid1, uuid2);
  EXPECT_TRUE(IsValidUUID(uuid1));
  EXPECT_TRUE(IsValidUUID(uuid2));
}

// 测试用例6：不同字符串生成不同UUID
TEST_F(UUIDTest, GenerateFromDifferentStrings)
{
  // 使用不同字符串生成UUID
  auto uuid1 = mite::UUIDGenerator::Generate("string1");
  auto uuid2 = mite::UUIDGenerator::Generate("string2");
  auto uuid3 = mite::UUIDGenerator::Generate("string3");

  // 验证所有UUID都是有效且不同的
  EXPECT_TRUE(IsValidUUID(uuid1));
  EXPECT_TRUE(IsValidUUID(uuid2));
  EXPECT_TRUE(IsValidUUID(uuid3));
  EXPECT_TRUE(AreUUIDsDifferent(uuid1, uuid2));
  EXPECT_TRUE(AreUUIDsDifferent(uuid1, uuid3));
  EXPECT_TRUE(AreUUIDsDifferent(uuid2, uuid3));
}

// 测试用例7：空字符串处理测试
TEST_F(UUIDTest, GenerateFromEmptyString)
{
  // 测试空字符串生成UUID
  auto uuid = mite::UUIDGenerator::Generate("");

  // 验证生成的UUID有效
  EXPECT_TRUE(IsValidUUID(uuid));

  // 验证多次生成空字符串得到相同结果
  auto uuid2 = mite::UUIDGenerator::Generate("");
  EXPECT_EQ(uuid, uuid2);
}

// 测试用例8：特殊字符字符串测试
TEST_F(UUIDTest, GenerateFromSpecialCharacters)
{
  const char *special_chars = "!@#$%^&*()_+-=[]{}|;:',.<>/?";

  // 测试特殊字符字符串生成UUID
  auto uuid1 = mite::UUIDGenerator::Generate(special_chars);
  auto uuid2 = mite::UUIDGenerator::Generate(special_chars);

  // 验证UUID有效且确定性
  EXPECT_TRUE(IsValidUUID(uuid1));
  EXPECT_TRUE(IsValidUUID(uuid2));
  EXPECT_EQ(uuid1, uuid2);
}

// 测试用例9：多线程安全性测试
TEST_F(UUIDTest, ThreadSafety)
{
  constexpr int NUM_THREADS = 10;
  constexpr int UUIDS_PER_THREAD = 100;

  std::vector<std::thread> threads;
  std::vector<std::vector<uuids::uuid>> thread_results(NUM_THREADS);
  std::atomic<int> completed_threads(0);

  // 创建多个线程同时生成UUID
  for (int i = 0; i < NUM_THREADS; ++i) {
    threads.emplace_back([i, &thread_results, &completed_threads, UUIDS_PER_THREAD]() {
      for (int j = 0; j < UUIDS_PER_THREAD; ++j) {
        thread_results[i].push_back(mite::UUIDGenerator::Generate());
      }
      completed_threads++;
    });
  }

  // 等待所有线程完成
  for (auto &thread : threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }

  // 验证所有线程都已完成
  EXPECT_EQ(completed_threads, NUM_THREADS);

  // 收集所有生成的UUID并验证唯一性
  std::unordered_set<std::string> all_uuids;
  for (const auto &thread_uuids : thread_results) {
    for (const auto &uuid : thread_uuids) {
      std::string uuid_str = uuids::to_string(uuid);
      EXPECT_TRUE(IsValidUUID(uuid));

      // 验证UUID唯一性
      EXPECT_EQ(all_uuids.find(uuid_str), all_uuids.end());
      all_uuids.insert(uuid_str);
    }
  }

  // 验证总UUID数量正确
  EXPECT_EQ(all_uuids.size(), NUM_THREADS * UUIDS_PER_THREAD);
}

// 测试用例10：性能测试 - 批量生成UUID
TEST_F(UUIDTest, PerformanceBatchGeneration)
{
  constexpr int BATCH_SIZE = 10000;
  std::vector<uuids::uuid> uuids;
  uuids.reserve(BATCH_SIZE);

  // 批量生成UUID并测量时间
  auto start_time = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < BATCH_SIZE; ++i) {
    uuids.push_back(mite::UUIDGenerator::Generate());
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  // 验证所有UUID有效且唯一
  std::unordered_set<std::string> uuid_set;
  for (const auto &uuid : uuids) {
    EXPECT_TRUE(IsValidUUID(uuid));
    std::string uuid_str = uuids::to_string(uuid);
    EXPECT_EQ(uuid_set.find(uuid_str), uuid_set.end());
    uuid_set.insert(uuid_str);
  }

  // 输出性能信息（在测试报告中可见）
  std::cout << "Generated " << BATCH_SIZE << " UUIDs in " << duration.count() << " ms"
            << std::endl;
  std::cout << "Average time per UUID: " << (duration.count() * 1000.0 / BATCH_SIZE) << " μs"
            << std::endl;

  // 验证生成了正确数量的UUID
  EXPECT_EQ(uuid_set.size(), BATCH_SIZE);
}

// 测试用例11：边界条件测试 - 极大索引值
TEST_F(UUIDTest, LargeIndexValue)
{
  const size_t large_index = std::numeric_limits<size_t>::max();

  // 测试极大索引值生成UUID
  auto uuid = mite::UUIDGenerator::Generate(large_index);

  // 验证UUID有效
  EXPECT_TRUE(IsValidUUID(uuid));

  // 验证相同索引生成相同UUID
  auto uuid2 = mite::UUIDGenerator::Generate(large_index);
  EXPECT_EQ(uuid, uuid2);
}

// 测试用例12：字符串哈希一致性测试
TEST_F(UUIDTest, StringHashConsistency)
{
  // 测试相同内容不同指针的字符串生成相同UUID
  const char *str1 = "hello_world";
  std::string str2 = "hello_world";

  auto uuid1 = mite::UUIDGenerator::Generate(str1);
  auto uuid2 = mite::UUIDGenerator::Generate(str2.c_str());

  // 验证相同内容的字符串生成相同UUID
  EXPECT_EQ(uuid1, uuid2);
  EXPECT_TRUE(IsValidUUID(uuid1));
  EXPECT_TRUE(IsValidUUID(uuid2));
}

// 测试用例13：UUID字符串转换测试
TEST_F(UUIDTest, UUIDStringConversion)
{
  // 生成UUID并测试字符串转换
  auto uuid = mite::UUIDGenerator::Generate();
  std::string uuid_str = uuids::to_string(uuid);

  // 验证字符串格式符合UUID标准
  EXPECT_EQ(uuid_str.length(), 36);  // UUID字符串长度应为36字符
  EXPECT_EQ(uuid_str[8], '-');
  EXPECT_EQ(uuid_str[13], '-');
  EXPECT_EQ(uuid_str[18], '-');
  EXPECT_EQ(uuid_str[23], '-');

  // 验证可以从字符串解析回UUID
  auto parsed_uuid = uuids::uuid::from_string(uuid_str);
  EXPECT_TRUE(parsed_uuid.has_value());
  EXPECT_EQ(uuid, parsed_uuid.value());
}

// 测试用例14：nil UUID 处理测试
TEST_F(UUIDTest, NonNilUUID)
{
  // 验证生成的UUID不是nil UUID
  auto uuid = mite::UUIDGenerator::Generate();
  EXPECT_FALSE(uuid.is_nil());

  // 验证基于索引的UUID也不是nil
  auto indexed_uuid = mite::UUIDGenerator::Generate(123);
  EXPECT_FALSE(indexed_uuid.is_nil());

  // 验证基于字符串的UUID也不是nil
  auto string_uuid = mite::UUIDGenerator::Generate("test");
  EXPECT_FALSE(string_uuid.is_nil());
}

// 测试用例15：版本号验证测试
TEST_F(UUIDTest, VersionNumberValidation)
{
  // 验证随机生成的UUID版本号为4（随机版本）
  auto random_uuid = mite::UUIDGenerator::Generate();
  EXPECT_EQ(random_uuid.version(), uuids::uuid_version::random_number_based);

  // 验证基于索引生成的UUID版本号也为4
  auto indexed_uuid = mite::UUIDGenerator::Generate(456);
  EXPECT_EQ(indexed_uuid.version(), uuids::uuid_version::random_number_based);

  // 验证基于字符串生成的UUID版本号也为4
  auto string_uuid = mite::UUIDGenerator::Generate("test_string");
  EXPECT_EQ(string_uuid.version(), uuids::uuid_version::random_number_based);
}