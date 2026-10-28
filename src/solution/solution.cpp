// You can remove or add any includes you need

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <rclcpp_components/register_node_macro.hpp>
#include <std_msgs/msg/int8.hpp>
#include <std_msgs/msg/u_int16.hpp>
#include <deque>
#include <algorithm>

namespace solution {

class Solution final : public rclcpp::Node {

public:
  explicit Solution(const rclcpp::NodeOptions& options = rclcpp::NodeOptions())
  : rclcpp::Node("solution", options) {

    RCLCPP_INFO(get_logger(), "%s", _preempt ? "Preempt" : "Queue");

    _motor1 = create_publisher<std_msgs::msg::Int8>("motor_1/control", rclcpp::QoS(10));
    _motor2 = create_publisher<std_msgs::msg::Int8>("motor_2/control", rclcpp::QoS(10));

    _enc1 = create_subscription<std_msgs::msg::UInt16>("motor_1/encoder", rclcpp::SensorDataQoS(),
      [this](const std_msgs::msg::UInt16::SharedPtr m) { _enc1_val = m->data; _have1 = true; });
    _enc2 = create_subscription<std_msgs::msg::UInt16>("motor_2/encoder", rclcpp::SensorDataQoS(),
      [this](const std_msgs::msg::UInt16::SharedPtr m) { _enc2_val = m->data; _have2 = true; });

    _target = create_subscription<geometry_msgs::msg::Point>("target",
      rclcpp::QoS(rclcpp::KeepLast(10)).reliable().transient_local(),
      [this](const geometry_msgs::msg::Point::SharedPtr point) {
        RCLCPP_INFO(get_logger(), "Target: (%g, %g, %g)", point->x, point->y, point->z);
          if (_preempt) _queue.clear();
          _queue.push_back(*point);
      });
    
    _timer = create_wall_timer(std::chrono::milliseconds(50), [this]() { control_loop(); });
  }

private:
  static constexpr int ticks = 4096;
  static constexpr int vibrations = 3;  
  static constexpr int MAX_SPEED = 127;
  static constexpr int MIN_SPEED = 50;
  static constexpr double TOP_DISTANCE = 300.0; 

  //żeby nie robić bezsensownego okrążenia
  static int set_path(int target, int current) {
    int dist = (target - current) % ticks;
    if (dist > ticks / 2) dist -= ticks; 
    if (dist <= -ticks / 2) dist += ticks;
    return dist;
  }

  static int to_ticks(double rad) {
    const long raw = std::lround(rad / (2 * M_PI) * ticks);
    const int t = static_cast<int>(raw % ticks);
    return t < 0 ? t + ticks : t;
  }


  static int8_t set_motion(int target, int current) {
    const int dist = set_path(target, current);
    if (std::abs(dist) <= vibrations) return 0;

    const double r = std::min(std::abs(dist) / TOP_DISTANCE, 1.0); // normalizacja 0..1
    const double s = std::sin(M_PI * r / 2.0);
    const int v = MIN_SPEED + static_cast<int>(s * (MAX_SPEED - MIN_SPEED));

    return static_cast<int8_t>(dist > 0 ? v : -v);
  }

  void control_loop() {
    std_msgs::msg::Int8 m1_motion, m2_motion;

    if (!_queue.empty() && _have1 && _have2) {

      const auto& p = _queue.front();

      const double horizontal_angle = -std::atan2(p.y, p.x);
      const double vertical_angle = std::atan2(p.z, std::hypot(p.x, p.y));

      const int target1 = to_ticks(horizontal_angle);
      const int target2 = to_ticks(vertical_angle);
      
      m1_motion.data = set_motion(target1, _enc1_val);
      m2_motion.data = set_motion(target2, _enc2_val);

      if (!_preempt && m1_motion.data == 0 && m2_motion.data == 0) {
        _queue.pop_front();
        if (!_queue.empty()) {
          const auto& p = _queue.front();
          RCLCPP_INFO(get_logger(), "Next goal: %g, %g, %g", p.x, p.y, p.z);
        }else{
          RCLCPP_INFO(get_logger(), "DONE AND DUSTED!");
        }
      }
    }

    _motor1->publish(m1_motion);
    _motor2->publish(m2_motion);

    
  }

  std::deque<geometry_msgs::msg::Point> _queue;
  bool _preempt{false};
  bool _have1{false}, _have2{false};
  int _enc1_val{0}, _enc2_val{0};

  rclcpp::Publisher<std_msgs::msg::Int8>::SharedPtr _motor1;
  rclcpp::Publisher<std_msgs::msg::Int8>::SharedPtr _motor2;
  rclcpp::Subscription<std_msgs::msg::UInt16>::SharedPtr _enc1;
  rclcpp::Subscription<std_msgs::msg::UInt16>::SharedPtr _enc2;
  rclcpp::Subscription<geometry_msgs::msg::Point>::SharedPtr _target;
  rclcpp::TimerBase::SharedPtr _timer;
};
}

RCLCPP_COMPONENTS_REGISTER_NODE(solution::Solution)