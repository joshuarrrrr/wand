#pragma once

#include <chrono>
#include <limits>
#include <vector>

namespace wand {

using Timestamp = std::chrono::time_point<std::chrono::high_resolution_clock>;

class TouchPoint {
 public:
  TouchPoint(int id);
  TouchPoint(const TouchPoint&) = delete;
  TouchPoint(TouchPoint&&) noexcept = default;
  ~TouchPoint() = default;

  TouchPoint& operator=(const TouchPoint&) = delete;
  TouchPoint& operator=(TouchPoint&&) noexcept = default;

  bool operator<(const TouchPoint& other) const;

  int id() const;
  bool active() const;

  const Timestamp& start_time() const;
  const Timestamp& update_time() const;
  const Timestamp& end_time() const;
  std::chrono::milliseconds duration() const;

  double start_x() const;
  double start_y() const;
  double x() const;
  double y() const;

  const std::vector<Timestamp>& timestamps() const;
  const std::vector<double>& x_positions() const;
  const std::vector<double>& y_positions() const;

  bool update(double x, double y);
  void finish();

 protected:
  int _id;
  bool _active;

  Timestamp _start_time;
  Timestamp _update_time;
  Timestamp _end_time;
  std::vector<Timestamp> _timestamps;
  std::vector<double> _x;
  std::vector<double> _y;
};

}  // namespace wand
