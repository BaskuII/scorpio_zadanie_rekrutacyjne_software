#include "motor.hpp"
#include <cmath>

backend::Motor::Motor(int8_t control_signal, uint16_t pose, std::optional<uint16_t> positive_limit,
  std::optional<uint16_t> negative_limit, double speed_factor, double inertia)
: _cs(control_signal), _pose(pose),
  _random_generator(std::random_device{}()),
  _normal_distribution(0.0, 0.6),
  _positive_limit(positive_limit),
  _negative_limit(negative_limit),
  _speed_factor(speed_factor),
  _inertia(inertia) { }

void backend::Motor::spin(double seconds) {
  const auto cs = static_cast<double>(_cs.load(std::memory_order_relaxed));
  const auto current_pose = _pose.load(std::memory_order_relaxed);
  // Inertia: the effective control signal follows the requested one with a first-order lag.
  const double alpha = _inertia > 0.0 ? 1.0 - std::exp(-seconds / _inertia) : 1.0;
  _effective_cs += (cs - _effective_cs) * alpha;
  // Speed is quadratic in the control signal. Fractional ticks are carried over to the next
  // spin so the speed does not depend on the encoder period.
  auto exact_delta = _effective_cs * _effective_cs * seconds * _speed_factor;
  if (_effective_cs < 0.0) {
    exact_delta = -exact_delta;
  }
  _residual += exact_delta;
  const auto delta = static_cast<int>(_residual);
  _residual -= delta;
  auto new_pose = current_pose + delta;
  if (_positive_limit.has_value() && delta > 0 && current_pose <= _positive_limit && new_pose > _positive_limit) {
    new_pose = _positive_limit.value();
    _residual = 0.0;
  }
  if (_negative_limit.has_value() && delta < 0 && current_pose >= _negative_limit && new_pose < _negative_limit) {
    new_pose = _negative_limit.value();
    _residual = 0.0;
  }
  while (new_pose < 0) {
    new_pose += 4096;
  }
  _pose.store(static_cast<uint16_t>(new_pose % 4096), std::memory_order_relaxed);
  const auto noise = static_cast<int>(std::round(_normal_distribution(_random_generator)));
  auto noisy_pose = (new_pose % 4096 + noise) % 4096;
  if (noisy_pose < 0) {
    noisy_pose += 4096;
  }
  if (_data_callback) {
    _data_callback(static_cast<uint16_t>(noisy_pose));
  }
}
