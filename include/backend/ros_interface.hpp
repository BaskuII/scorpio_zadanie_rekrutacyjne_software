#pragma once


#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include <geometry_msgs/msg/point.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/int8.hpp>
#include <std_msgs/msg/u_int16.hpp>

#include "basic_tester.hpp"
#include "tester.hpp"

namespace backend {

class RosInterface final : public rclcpp::Node {
public:
  explicit RosInterface(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

private:
  using MotorComponent = backend_interface::Component<int8_t, uint16_t>;

  rclcpp::Subscription<std_msgs::msg::Int8>::SharedPtr subscribe_control(
    const char* topic, std::shared_ptr<MotorComponent> motor);

  std::shared_ptr<BasicTester> _tester;

  rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr _motor1_encoder;
  rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr _motor2_encoder;
  rclcpp::Publisher<geometry_msgs::msg::Point>::SharedPtr _target;

  rclcpp::Subscription<std_msgs::msg::Int8>::SharedPtr _motor1_control;
  rclcpp::Subscription<std_msgs::msg::Int8>::SharedPtr _motor2_control;
};

}  // namespace backend
