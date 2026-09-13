#pragma once

#include <atomic>
#include <cstdint>
#include <optional>
#include <random>
#include <utility>
#include "component.hpp"
#include "spinnable.hpp"

namespace backend {
class Motor final : public backend_interface::Component<int8_t, uint16_t>, public Spinnable {
  #ifdef FRIEND_TEST
    FRIEND_TEST(MotorTest, LimitTest);
    FRIEND_TEST(MotorTest, LimitTest2);
    FRIEND_TEST(MotorTest, SpeedIndependentOfPeriod);
    FRIEND_TEST(MotorTest, Inertia);
    FRIEND_TEST(MotorTest, NoiseStaysInRange);
  #endif
  friend class BasicTester;
  std::atomic<int8_t> _cs;
  std::atomic<uint16_t> _pose;
  std::mt19937 _random_generator;
  std::normal_distribution<double> _normal_distribution;
  std::function<void(const uint16_t&)> _data_callback;
  std::optional<uint16_t> _positive_limit;
  std::optional<uint16_t> _negative_limit;
  // Motor characteristics: speed = effective_cs^2 * speed_factor [ticks/s],
  // effective_cs follows the requested control signal with a first-order lag
  // of time constant `inertia` seconds (0 = reacts instantly).
  double _speed_factor;
  double _inertia;
  // Spinner-thread-only state.
  double _effective_cs = 0.0;
  double _residual = 0.0;

public:
  static constexpr double default_speed_factor = 0.006;
  static constexpr double default_inertia = 0.0;

  Motor(int8_t control_signal, uint16_t pose, std::optional<uint16_t> positive_limit = std::nullopt,
    std::optional<uint16_t> negative_limit = std::nullopt, double speed_factor = default_speed_factor,
    double inertia = default_inertia);

  void spin(double) override;
  inline void add_data_callback(std::function<void(const uint16_t&)> data_callback) override  {
    _data_callback = std::move(data_callback);
  }
  inline void send_data(const int8_t& control_signal) override {
    _cs.store(control_signal, std::memory_order_relaxed);
  }
};
}  // namespace backend
