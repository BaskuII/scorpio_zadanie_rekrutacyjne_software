#include <chrono>
#include <iostream>
#include <utility>
#include "basic_tester.hpp"

namespace {
// Motor 1 (horizontal axis) carries the whole head: slower and heavier.
constexpr double motor1_speed_factor = 0.006;
constexpr double motor1_inertia = 0.5;   // seconds
// Motor 2 (vertical axis) moves only the camera: quicker and lighter.
constexpr double motor2_speed_factor = 0.008;
constexpr double motor2_inertia = 0.25;  // seconds
}  // namespace

backend::BasicTester::BasicTester(double encoder_period, std::vector<std::pair<double, Point>> signals,
  bool debug, Constraints constraints, bool autostart)
  : _motor1(std::make_shared<backend::Motor>(0, 0, constraints.horizontal_max, constraints.horizontal_min,
      motor1_speed_factor, motor1_inertia)),
    _motor2(std::make_shared<backend::Motor>(0, 0, constraints.vertical_max, constraints.vertical_min,
      motor2_speed_factor, motor2_inertia)),
    _commands(std::make_shared<backend::MockComponent<backend_interface::Tester::Impossible, Point>>()),
    _stop(false),
    _encoder_period(encoder_period),
    _debug(debug),
    _signals(std::move(signals)) {
  if (autostart) {
    start();
  }
}

backend::BasicTester::~BasicTester() {
  {
    std::lock_guard<std::mutex> lock(_stop_mutex);
    _stop.store(true, std::memory_order_relaxed);
  }
  _stop_cv.notify_all();
  if (_sender.joinable()) {
    _sender.join();
  }
  if (_spinner.joinable()) {
    _spinner.join();
  }
}

void backend::BasicTester::start() {
  if (_spinner.joinable()) {
    return;
  }
  _spinner = std::thread(&BasicTester::spinner, this, _encoder_period, _debug);
  _sender = std::thread(&BasicTester::sender, this, std::move(_signals));
}

void backend::BasicTester::spinner(const double encoder_period, const bool debug) {
  const auto frame_duration = std::chrono::duration<double>(encoder_period);
  while (!_stop.load(std::memory_order_relaxed)) {
    const auto start = std::chrono::steady_clock::now();
    _motor1->spin(static_cast<double>(frame_duration.count()));
    _motor2->spin(static_cast<double>(frame_duration.count()));
    if (debug) {
      std::cout << '[' << encoder_period << "] " << std::to_string(_motor1->_cs.load(std::memory_order_relaxed)) << " "
      << _motor1->_pose.load(std::memory_order_relaxed) << " "
      << std::to_string(_motor2->_cs.load(std::memory_order_relaxed)) << " "
      << _motor2->_pose.load(std::memory_order_relaxed) << std::endl;
    }
    const auto end = std::chrono::steady_clock::now();
    const auto elapsed = end - start;
    if (elapsed > frame_duration) {
      std::cerr << "Spinner overrun - callbacks took "
        << std::chrono::duration<double>(elapsed).count() << " s, encoder period is "
        << encoder_period << " s\n";
      continue;
    }
    std::unique_lock<std::mutex> lock(_stop_mutex);
    _stop_cv.wait_for(lock, frame_duration - elapsed, [this]() { return _stop.load(std::memory_order_relaxed); });
  }
}

void backend::BasicTester::sender(std::vector<std::pair<double, Point>> signals) {
  for (size_t i = 0; !_stop.load(std::memory_order_relaxed) && i < signals.size(); ++i) {
    const auto [delay, point] = signals[i];
    std::unique_lock<std::mutex> lock(_stop_mutex);
    _stop_cv.wait_for(lock, std::chrono::duration<double>(delay),
      [this]() { return _stop.load(std::memory_order_relaxed); });
    if (_stop.load(std::memory_order_relaxed)) {
      break;
    }
    lock.unlock();
    _commands->send_to_callback(point);
  }
}
