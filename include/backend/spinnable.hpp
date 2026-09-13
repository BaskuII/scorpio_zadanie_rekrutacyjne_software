#pragma once

namespace backend {
class Spinnable {
public:

  virtual ~Spinnable() = default;
  virtual void spin(double) = 0;
};
}  // namespace backend
