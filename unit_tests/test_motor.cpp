#include <gtest/gtest.h>
#include <gtest/gtest_prod.h>
#include "motor.hpp"

namespace backend {
TEST(MotorTest, LimitTest) {
  backend::Motor motor(0, 2048, 4000, 10);
  EXPECT_LE(motor._pose.load(std::memory_order_relaxed) - 2048, 4);
  motor.send_data(100);
  for (int i = 0; i < 1000; ++i) {
    motor.spin(0.1);
  }
  EXPECT_LT(motor._pose.load(std::memory_order_relaxed) - 4000, 4);
  for (int i = 0; i < 1000; ++i) {
    motor.spin(0.1);
  }
  motor.send_data(-100);
  for (int i = 0; i < 1000; ++i) {
    motor.spin(0.1);
  }
  EXPECT_LT(motor._pose.load(std::memory_order_relaxed) - 10, 4);
}

TEST(MotorTest, LimitTest2) {
  backend::Motor motor(10, 10, 30, 4000);
  EXPECT_LE(motor._pose.load(std::memory_order_relaxed) - 10, 4);
  motor.send_data(100);
  for (int i = 0; i < 1000; ++i) {
    motor.spin(0.1);
  }
  EXPECT_LT(motor._pose.load(std::memory_order_relaxed) - 30, 4);
  for (int i = 0; i < 1000; ++i) {
    motor.spin(0.1);
  }
  motor.send_data(-100);
  for (int i = 0; i < 1000; ++i) {
    motor.spin(0.1);
  }
  EXPECT_LT(motor._pose.load(std::memory_order_relaxed) - 4000, 4);
}

TEST(MotorTest, SpeedIndependentOfPeriod) {
  // 5 seconds of driving at 100 must cover the same distance no matter how often spin() is called.
  backend::Motor slow_ticks(100, 0);
  backend::Motor fast_ticks(100, 0);
  for (int i = 0; i < 100; ++i) {
    slow_ticks.spin(0.05);
  }
  for (int i = 0; i < 500; ++i) {
    fast_ticks.spin(0.01);
  }
  const int expected = static_cast<int>(100 * 100 * 5.0 * backend::Motor::default_speed_factor);
  EXPECT_NEAR(slow_ticks._pose.load(std::memory_order_relaxed), expected, 1);
  EXPECT_NEAR(fast_ticks._pose.load(std::memory_order_relaxed), expected, 1);
}

TEST(MotorTest, Inertia) {
  backend::Motor instant(100, 0, std::nullopt, std::nullopt, backend::Motor::default_speed_factor, 0.0);
  backend::Motor lazy(100, 0, std::nullopt, std::nullopt, backend::Motor::default_speed_factor, 0.5);
  for (int i = 0; i < 10; ++i) {
    instant.spin(0.05);
    lazy.spin(0.05);
  }
  // After half a second the lazy motor is still accelerating, so it has covered less distance.
  EXPECT_LT(lazy._pose.load(std::memory_order_relaxed), instant._pose.load(std::memory_order_relaxed));
  // ... and it keeps rolling for a while after the signal is cut.
  lazy.send_data(0);
  const auto before_stop = lazy._pose.load(std::memory_order_relaxed);
  for (int i = 0; i < 10; ++i) {
    lazy.spin(0.05);
  }
  EXPECT_GT(lazy._pose.load(std::memory_order_relaxed), before_stop);
}

TEST(MotorTest, NoiseStaysInRange) {
  // A motor parked at 0 must report readings that wrap correctly (4093..4095 or 0..3), never garbage.
  backend::Motor motor(0, 0);
  motor.add_data_callback([](const uint16_t& reading) {
    EXPECT_TRUE(reading <= 3 || reading >= 4093) << "reading: " << reading;
  });
  for (int i = 0; i < 1000; ++i) {
    motor.spin(0.05);
  }
}
}  // namespace backend
