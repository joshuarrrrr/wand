#include <touch_point.hpp>

namespace wand {

TouchPoint::TouchPoint(int id) : _id{id}, _active{true}, _start_time{std::chrono::high_resolution_clock::now()} {
  _update_time = _start_time;
}

bool TouchPoint::operator<(const TouchPoint& other) const {
  return _id < other.id();
}

int TouchPoint::id() const {
  return _id;
}

bool TouchPoint::active() const {
  return _active;
}

const Timestamp& TouchPoint::start_time() const {
  return _start_time;
}

const Timestamp& TouchPoint::update_time() const {
  return _update_time;
}

const Timestamp& TouchPoint::end_time() const {
  return _end_time;
}

std::chrono::milliseconds TouchPoint::duration() const {
  if (!active()) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(update_time() - start_time());
  }
  return std::chrono::duration_cast<std::chrono::milliseconds>(end_time() - start_time());
}

double TouchPoint::start_x() const {
  if (_x.empty()) {
    return std::numeric_limits<double>::min();
  }

  return _x.front();
}

double TouchPoint::start_y() const {
  if (_y.empty()) {
    return std::numeric_limits<double>::min();
  }

  return _y.front();
}

double TouchPoint::x() const {
  if (_x.empty()) {
    return std::numeric_limits<double>::min();
  }

  return _x.back();
}

double TouchPoint::y() const {
  if (_y.empty()) {
    return std::numeric_limits<double>::min();
  }

  return _y.back();
}

const std::vector<Timestamp>& TouchPoint::timestamps() const {
  return _timestamps;
}

const std::vector<double>& TouchPoint::x_positions() const {
  return _x;
}

const std::vector<double>& TouchPoint::y_positions() const {
  return _y;
}

bool TouchPoint::update(double x, double y) {
  // first update has to contain both position values
  if ((_x.empty() || _y.empty()) && (x < 0 || y < 0)) {
    return false;
  }

  // add current time as time stamp
  _update_time = std::chrono::high_resolution_clock::now();
  _timestamps.push_back(_update_time);

  // update x if positive value was passed
  if (x < 0) {
    _x.push_back(this->x());
  } else {
    _x.push_back(x);
  }

  // update y if positive value was passed
  if (y < 0) {
    _y.push_back(this->y());
  } else {
    _y.push_back(y);
  }

  return true;
}

void TouchPoint::finish() {
  _active = false;
  _end_time = std::chrono::high_resolution_clock::now();
}

}  // namespace wand