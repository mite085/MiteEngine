#include "time/time.h"
#include "gtest/gtest.h"
#include <chrono>
#include <thread>
#include <cmath>

namespace mite_test {
// Time类测试夹具
class TimeTest : public ::testing::Test {
 protected:
  // 每个测试前重置时间状态
  void SetUp() override
  { 
    mite::Time::Reset();
  }
  // 辅助函数：等待指定毫秒数
  void WaitMilliseconds(int ms)
  {
    auto start = std::chrono::high_resolution_clock::now();
    while (true) {
      auto now = std::chrono::high_resolution_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
      if (elapsed.count() >= ms)
        break;
    }
  }
};

// 测试用例1：基本功能测试 - 时间更新和获取
TEST_F(TimeTest, BasicTimeFunctionality)
{
  // 第一次更新时间
  mite::Time::Update();

  float deltaTimeSec = mite::Time::DeltaTime();
  size_t deltaTimeMS = mite::Time::DeltaTimeMS();
  float currentTimeSec = mite::Time::CurrentTime();
  size_t currentTimeMS = mite::Time::CurrentTimeMS();
  // 验证初始值合理性
  EXPECT_GE(deltaTimeSec, 0.0f);
  EXPECT_LE(deltaTimeSec, 0.1f);
  EXPECT_GE(deltaTimeMS, 0u);
  EXPECT_LE(deltaTimeMS, 100u);
  EXPECT_GE(currentTimeSec, 0.0f);
  EXPECT_GE(currentTimeMS, 0u);
  // 验证两种精度的一致性
  EXPECT_NEAR(deltaTimeSec * 1000.0f, static_cast<float>(deltaTimeMS), 1.0f);
  EXPECT_NEAR(currentTimeSec * 1000.0f, static_cast<float>(currentTimeMS), 1.0f);
}
// 测试用例2：时间单调递增测试
TEST_F(TimeTest, TimeIncreasesMonotonically)
{
  // 获取初始时间
  float initialTimeSec = mite::Time::CurrentTime();
  size_t initialTimeMS = mite::Time::CurrentTimeMS();
  // 等待一段时间后更新
  WaitMilliseconds(50);
  mite::Time::Update();
  float laterTimeSec = mite::Time::CurrentTime();
  size_t laterTimeMS = mite::Time::CurrentTimeMS();
  // 验证时间单调递增
  EXPECT_GT(laterTimeSec, initialTimeSec);
  EXPECT_GT(laterTimeMS, initialTimeMS);
  // 验证时间增加量合理
  float timeIncreaseSec = laterTimeSec - initialTimeSec;
  size_t timeIncreaseMS = laterTimeMS - initialTimeMS;

  EXPECT_GE(timeIncreaseSec, 0.04f);  // 至少40ms
  EXPECT_LE(timeIncreaseSec, 0.06f);  // 最多60ms
  EXPECT_GE(timeIncreaseMS, 40u);     // 至少40ms
  EXPECT_LE(timeIncreaseMS, 60u);     // 最多60ms
}
// 测试用例3：DeltaTime准确性测试
TEST_F(TimeTest, DeltaTimeAccuracy)
{
  const size_t waitTimeMS = 80;

  mite::Time::Update();
  // 等待指定时间
  WaitMilliseconds(waitTimeMS);
  mite::Time::Update();
  float deltaTime = mite::Time::DeltaTime();
  size_t deltaTimeMS = mite::Time::DeltaTimeMS();
  // 验证DeltaTime反映了等待的时间
  EXPECT_GE(deltaTime, waitTimeMS / 1000.0f * 0.8f);  // 至少80%
  EXPECT_LE(deltaTime, waitTimeMS / 1000.0f * 1.2f);  // 最多120%
  EXPECT_GE(deltaTimeMS, waitTimeMS - 10u);           // 至少减少10ms容差
  EXPECT_LE(deltaTimeMS, waitTimeMS + 10u);           // 最多增加10ms容差
}
// 测试用例4：时间精度一致性测试
TEST_F(TimeTest, TimePrecisionConsistency)
{
  mite::Time::Update();

  // 获取两种精度的时间
  float timeSec1 = mite::Time::CurrentTime();
  size_t timeMS1 = mite::Time::CurrentTimeMS();

  // 快速连续更新
  for (int i = 0; i < 5; ++i) {
    WaitMilliseconds(1);
    mite::Time::Update();
  }

  float timeSec2 = mite::Time::CurrentTime();
  size_t timeMS2 = mite::Time::CurrentTimeMS();
  // 验证时间略有增加
  EXPECT_GT(timeSec2, timeSec1);
  EXPECT_GT(timeMS2, timeMS1);
  // 验证增加量很小
  EXPECT_LT(timeSec2 - timeSec1, 0.01f);
  EXPECT_LT(timeMS2 - timeMS1, 10u);
  // 验证两种精度保持同步
  float calculatedMS = (timeSec2 - timeSec1) * 1000.0f;
  size_t actualMS = timeMS2 - timeMS1;
  EXPECT_NEAR(calculatedMS, static_cast<float>(actualMS), 1.0f);
}
// 测试用例5：极大时间值处理测试
TEST_F(TimeTest, DeltaTimeClamping)
{
  // 直接暂停1秒（正常情况下每帧更新，不太可能超出1秒）
  WaitMilliseconds(1000);
  mite::Time::Update();

  float secondDelta = mite::Time::DeltaTime();
  size_t secondDeltaMS = mite::Time::DeltaTimeMS();

  // 验证时间累积正确
  EXPECT_GE(secondDelta, 0.999f);   // 至少999ms
  EXPECT_LE(secondDelta, 1.001f);   // 最多1001ms
  EXPECT_GE(secondDeltaMS, 999u);   // 至少999ms
  EXPECT_LE(secondDeltaMS, 1001u);  // 最多1001ms
}
// 测试用例6：时间重置行为测试
TEST_F(TimeTest, TimeResetBehavior)
{
  // 等待30毫秒并更新
  WaitMilliseconds(30);
  mite::Time::Update();

  // 执行初始化
  mite::Time::Reset();
  mite::Time::Update();
  // 验证状态已更新
  EXPECT_LT(mite::Time::CurrentTime(), 0.001f);
  EXPECT_LT(mite::Time::CurrentTimeMS(), 1u);
}
// 测试用例7：长时间运行稳定性测试
TEST_F(TimeTest, LongRunningStability)
{
  const int NUM_UPDATES = 100;
  float totalTime = 0.0f;
  size_t totalTimeMS = 0;
  for (int i = 0; i < NUM_UPDATES; ++i) {
    WaitMilliseconds(10);
    mite::Time::Update();

    totalTime += mite::Time::DeltaTime();
    totalTimeMS += mite::Time::DeltaTimeMS();
    // 每次更新后验证一致性
    EXPECT_NEAR(mite::Time::CurrentTime() * 1000.0f,
                static_cast<float>(mite::Time::CurrentTimeMS()),
                2.0f);
  }
  // 验证总时间累积正确（百分之一误差的容许度）
  EXPECT_GE(totalTime, 0.990f);     // 至少990ms
  EXPECT_LE(totalTime, 1.010f);     // 最多1010ms
  EXPECT_GE(totalTimeMS, 990u);   // 至少990ms
  EXPECT_LE(totalTimeMS, 1010u);  // 最多1010ms
  // 验证两种精度的一致性
  EXPECT_NEAR(totalTime * 1000.0f, static_cast<float>(totalTimeMS), 10.0f);
}

// 测试用例8：API调用一致性测试
TEST_F(TimeTest, APICallConsistency)
{
  // 验证多次调用返回相同值（直到下一次Update）
  mite::Time::Update();

  float delta1 = mite::Time::DeltaTime();
  size_t deltaMS1 = mite::Time::DeltaTimeMS();
  float current1 = mite::Time::CurrentTime();
  size_t currentMS1 = mite::Time::CurrentTimeMS();

  // 再次调用应该返回相同值
  float delta2 = mite::Time::DeltaTime();
  size_t deltaMS2 = mite::Time::DeltaTimeMS();
  float current2 = mite::Time::CurrentTime();
  size_t currentMS2 = mite::Time::CurrentTimeMS();

  EXPECT_FLOAT_EQ(delta1, delta2);
  EXPECT_EQ(deltaMS1, deltaMS2);
  EXPECT_FLOAT_EQ(current1, current2);
  EXPECT_EQ(currentMS1, currentMS2);

  // 更新后值应该变化
  WaitMilliseconds(20);
  mite::Time::Update();

  EXPECT_NE(delta1, mite::Time::DeltaTime());
  EXPECT_NE(deltaMS1, mite::Time::DeltaTimeMS());
  EXPECT_GT(mite::Time::CurrentTime(), current1);
  EXPECT_GT(mite::Time::CurrentTimeMS(), currentMS1);
}
// 测试用例9：混合精度数学运算测试
TEST_F(TimeTest, MixedPrecisionMathOperations)
{
  // 测试在实际使用场景中的混合精度运算

  const float velocity = 5.0f;  // 米/秒

  mite::Time::Update();

  // 等待0.2秒
  WaitMilliseconds(200);
  mite::Time::Update();

  // 使用秒精度计算位移
  float displacementSec = velocity * mite::Time::DeltaTime();

  // 使用毫秒精度计算位移
  float displacementMS = velocity * (mite::Time::DeltaTimeMS() / 1000.0f);

  // 两种方法应该得到近似结果
  EXPECT_NEAR(displacementSec, displacementMS, 0.001f);

  // 验证位移量合理
  EXPECT_GE(displacementSec, 0.9f);  // 至少0.9米
  EXPECT_LE(displacementSec, 1.1f);  // 最多1.1米
}
}  // namespace mite_test