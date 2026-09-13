#pragma once

#include <functional>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>
#include "component.hpp"

namespace backend {
template<typename T, typename U>
class MockComponent final : public backend_interface::Component<T, U> {
  std::function<void(const U&)> _data_callback;
  std::mutex _mutex;
  std::vector<T> _sent_data;

public:
  void add_data_callback(std::function<void(const U&)> callback) override {
    _data_callback = std::move(callback);
  }

  void send_data(const T& value) override {
    if constexpr (std::is_constructible_v<T>) {
      std::lock_guard<std::mutex> lock(_mutex);
      _sent_data.push_back(value);
    }
  }

  void send_to_callback(const U& value) {
    if (_data_callback) {
      _data_callback(value);
    }
  }

  template<typename V = T>
  std::enable_if_t<std::is_constructible_v<V>, std::vector<T>> get_sent_data() {
    std::vector<T> ans;
    ans.reserve(16);
    std::lock_guard<std::mutex> lock(_mutex);
    std::swap(ans, _sent_data);
    return ans;
  }
};
}  // namespace backend
