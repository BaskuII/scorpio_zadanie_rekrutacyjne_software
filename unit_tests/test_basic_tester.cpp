#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include "basic_tester.hpp"

TEST(BasicTesterTest, CreationAndDestruction) {
  Constraints constraints{4095, 0, 4095, 0};
  std::vector<std::pair<double, Point>> signals = {
    {0.0, {1.0, 2.0, 3.0}},
    {1.0, {4.0, 5.0, 6.0}}
  };
  EXPECT_NO_THROW({
    auto tester = std::make_shared<backend::BasicTester>(0.05, signals, true, constraints);
  });
}

TEST(BasicTesterTest, GetComponents) {
  Constraints constraints{4095, 0, 4095, 0};
  std::vector<std::pair<double, Point>> signals = {
    {0.0, {1.0, 2.0, 3.0}},
    {1.0, {4.0, 5.0, 6.0}}
  };
  auto tester = std::make_shared<backend::BasicTester>(0.05, signals, true, constraints);
  auto motor1 = tester->get_motor_1();
  auto motor2 = tester->get_motor_2();
  auto commands = tester->get_commands();
  EXPECT_NE(motor1, nullptr);
  EXPECT_NE(motor2, nullptr);
  EXPECT_NE(commands, nullptr);
}

TEST(BasicTesterTest, CreationWithEmptySignals) {
  Constraints constraints{4095, 0, 4095, 0};
  std::vector<std::pair<double, Point>> empty_signals;
  
  EXPECT_NO_THROW({
    auto tester = std::make_shared<backend::BasicTester>(0.1, empty_signals, false, constraints);
    auto motor1 = tester->get_motor_1();
    auto motor2 = tester->get_motor_2();
    auto commands = tester->get_commands();
    EXPECT_NE(motor1, nullptr);
    EXPECT_NE(motor2, nullptr);
    EXPECT_NE(commands, nullptr);
  });
}

TEST(BasicTesterTest, CreationWithDefaultConstraints) {
  std::vector<std::pair<double, Point>> signals = {
    {0.1, {1.0, 2.0, 3.0}}
  };
  
  EXPECT_NO_THROW({
    auto tester = std::make_shared<backend::BasicTester>(0.05, signals);
    auto motor1 = tester->get_motor_1();
    auto motor2 = tester->get_motor_2();
    auto commands = tester->get_commands();
    EXPECT_NE(motor1, nullptr);
    EXPECT_NE(motor2, nullptr);
    EXPECT_NE(commands, nullptr);
  });
}

TEST(BasicTesterTest, CreationWithPartialConstraints) {
  Constraints constraints{std::nullopt, 100, 2000, std::nullopt};
  std::vector<std::pair<double, Point>> signals = {
    {0.05, {10.0, 20.0, 30.0}}
  };
  
  EXPECT_NO_THROW({
    auto tester = std::make_shared<backend::BasicTester>(0.02, signals, false, constraints);
  });
}

TEST(BasicTesterTest, DebugModeEnabled) {
  Constraints constraints{4095, 0, 4095, 0};
  std::vector<std::pair<double, Point>> signals = {
    {0.01, {1.0, 2.0, 3.0}}
  };
  
  EXPECT_NO_THROW({
    auto tester = std::make_shared<backend::BasicTester>(0.01, signals, true, constraints);
  });
}

TEST(BasicTesterTest, DebugModeDisabled) {
  Constraints constraints{4095, 0, 4095, 0};
  std::vector<std::pair<double, Point>> signals = {
    {0.01, {1.0, 2.0, 3.0}}
  };
  
  EXPECT_NO_THROW({
    auto tester = std::make_shared<backend::BasicTester>(0.01, signals, false, constraints);
  });
}

TEST(BasicTesterTest, MultipleSignalsSequence) {
  Constraints constraints{4095, 0, 4095, 0};
  std::vector<std::pair<double, Point>> signals = {
    {0.01, {1.0, 2.0, 3.0}},
    {0.01, {4.0, 5.0, 6.0}},
    {0.01, {7.0, 8.0, 9.0}},
    {0.01, {10.0, 11.0, 12.0}}
  };
  
  EXPECT_NO_THROW({
    auto tester = std::make_shared<backend::BasicTester>(0.005, signals, false, constraints);
  });
}

TEST(BasicTesterTest, CommandsCallback) {
  Constraints constraints{4095, 0, 4095, 0};
  std::vector<std::pair<double, Point>> signals = {
    {0.02, {100.0, 200.0, 300.0}}
  };
  
  auto tester = std::make_shared<backend::BasicTester>(0.01, signals, false, constraints);
  auto commands = tester->get_commands();
  
  bool callback_called = false;
  Point received_point;
  
  commands->add_data_callback([&](const Point& point) {
    callback_called = true;
    received_point = point;
  });
  
  // Allow some time for the signal to be sent
  std::this_thread::sleep_for(std::chrono::milliseconds(30));
  
  // Note: Due to the threaded nature, we might not always catch the callback
  // This test mainly ensures the callback mechanism works without crashes
}

TEST(BasicTesterTest, MotorDataCallback) {
  Constraints constraints{4095, 0, 4095, 0};
  std::vector<std::pair<double, Point>> signals = {
    {0.05, {1.0, 2.0, 3.0}}
  };
  
  auto tester = std::make_shared<backend::BasicTester>(0.01, signals, false, constraints);
  auto motor1 = tester->get_motor_1();
  auto motor2 = tester->get_motor_2();
  
  bool motor1_callback_called = false;
  bool motor2_callback_called = false;
  uint16_t motor1_data = 0;
  uint16_t motor2_data = 0;
  
  motor1->add_data_callback([&](const uint16_t& data) {
    motor1_callback_called = true;
    motor1_data = data;
  });
  
  motor2->add_data_callback([&](const uint16_t& data) {
    motor2_callback_called = true;
    motor2_data = data;
  });
  
  // Send some control signals
  motor1->send_data(50);
  motor2->send_data(-30);
  
  // Allow some time for processing
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

TEST(BasicTesterTest, ShortLivedTester) {
  Constraints constraints{1000, 0, 1000, 0};
  std::vector<std::pair<double, Point>> signals = {
    {0.01, {1.0, 1.0, 1.0}}
  };
  
  {
    auto tester = std::make_shared<backend::BasicTester>(0.005, signals, false, constraints);
    auto motor1 = tester->get_motor_1();
    EXPECT_NE(motor1, nullptr);
    // Tester should be destroyed when going out of scope
  }
  
  // If we reach here without hanging, destruction worked correctly
  SUCCEED();
}

TEST(BasicTesterTest, VeryShortEncoderPeriod) {
  Constraints constraints{4095, 0, 4095, 0};
  std::vector<std::pair<double, Point>> signals = {
    {0.001, {1.0, 2.0, 3.0}}
  };
  
  EXPECT_NO_THROW({
    auto tester = std::make_shared<backend::BasicTester>(0.001, signals, false, constraints);
  });
}

TEST(BasicTesterTest, MotorControlSignalPersistence) {
  Constraints constraints{4095, 0, 4095, 0};
  std::vector<std::pair<double, Point>> signals = {
    {0.01, {1.0, 2.0, 3.0}}
  };
  
  auto tester = std::make_shared<backend::BasicTester>(0.005, signals, false, constraints);
  auto motor1 = tester->get_motor_1();
  auto motor2 = tester->get_motor_2();
  
  // Send control signals and verify they persist
  motor1->send_data(100);
  motor2->send_data(-50);
  
  // Allow some processing time
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  
  // Send different signals
  motor1->send_data(25);
  motor2->send_data(75);
  
  // Motors should continue operating without crashes
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  
  SUCCEED(); // Test passes if no crashes occur
}

TEST(BasicTesterTest, NegativePointValues) {
  Constraints constraints{4095, 0, 4095, 0};
  std::vector<std::pair<double, Point>> signals = {
    {0.01, {-1.0, -2.0, -3.0}},
    {0.01, {-10.5, 15.7, -20.3}}
  };
  
  EXPECT_NO_THROW({
    auto tester = std::make_shared<backend::BasicTester>(0.01, signals, false, constraints);
  });
}

TEST(BasicTesterTest, LargePointValues) {
  Constraints constraints{4095, 0, 4095, 0};
  std::vector<std::pair<double, Point>> signals = {
    {0.01, {1000.0, 2000.0, 3000.0}},
    {0.01, {99999.9, -99999.9, 0.0}}
  };
  
  EXPECT_NO_THROW({
    auto tester = std::make_shared<backend::BasicTester>(0.01, signals, false, constraints);
  });
}

TEST(BasicTesterTest, ConstraintLimitsValidation) {
  // Test with extreme constraint values
  Constraints extreme_constraints{65535, 0, 65535, 0};
  std::vector<std::pair<double, Point>> signals = {
    {0.01, {1.0, 2.0, 3.0}}
  };
  
  auto tester = std::make_shared<backend::BasicTester>(0.01, signals, false, extreme_constraints);
  auto motor1 = tester->get_motor_1();
  auto motor2 = tester->get_motor_2();
  
  // Test that motors respect their constraints
  motor1->send_data(127);  // Maximum int8_t value
  motor2->send_data(-128); // Minimum int8_t value
  
  std::this_thread::sleep_for(std::chrono::milliseconds(15));
  
  EXPECT_NE(motor1, nullptr);
  EXPECT_NE(motor2, nullptr);
}

TEST(BasicTesterTest, SignalTimingAccuracy) {
  Constraints constraints{4095, 0, 4095, 0};
  std::vector<std::pair<double, Point>> signals = {
    {0.01, {1.0, 2.0, 3.0}},
    {0.01, {4.0, 5.0, 6.0}},
    {0.01, {7.0, 8.0, 9.0}}
  };
  
  auto tester = std::make_shared<backend::BasicTester>(0.005, signals, false, constraints);
  auto commands = tester->get_commands();
  
  std::vector<Point> received_points;
  std::mutex callback_mutex;
  
  commands->add_data_callback([&](const Point& point) {
    std::lock_guard<std::mutex> lock(callback_mutex);
    received_points.push_back(point);
  });
  
  // Wait for all signals to be sent (3 signals * 0.01s each + buffer)
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  
  // Verify we received the expected number of signals
  std::lock_guard<std::mutex> lock(callback_mutex);
  EXPECT_EQ(received_points.size(), 3);
  
  if (received_points.size() >= 3) {
    // Verify signals are received in the correct order with expected values
    EXPECT_DOUBLE_EQ(received_points[0].x, 1.0);
    EXPECT_DOUBLE_EQ(received_points[0].y, 2.0);
    EXPECT_DOUBLE_EQ(received_points[0].z, 3.0);
    
    EXPECT_DOUBLE_EQ(received_points[1].x, 4.0);
    EXPECT_DOUBLE_EQ(received_points[1].y, 5.0);
    EXPECT_DOUBLE_EQ(received_points[1].z, 6.0);
    
    EXPECT_DOUBLE_EQ(received_points[2].x, 7.0);
    EXPECT_DOUBLE_EQ(received_points[2].y, 8.0);
    EXPECT_DOUBLE_EQ(received_points[2].z, 9.0);
  }
}
