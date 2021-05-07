#pragma once

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <atomic>
#include <limits>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <libevdev/libevdev.h>

#include <touch_point.hpp>

namespace wand {

using TouchPtr = std::shared_ptr<TouchPoint>;
using TouchPtrSet = std::set<TouchPtr>;

class MultitouchDevice {
 public:
  MultitouchDevice(const std::string& path);
  MultitouchDevice(const MultitouchDevice&) = delete;
  MultitouchDevice(MultitouchDevice&&) noexcept = default;
  ~MultitouchDevice();

  MultitouchDevice& operator=(const MultitouchDevice&) = delete;
  MultitouchDevice& operator=(MultitouchDevice&&) noexcept = default;

  const std::string& name() const;
  int num_slots() const;
  std::set<std::shared_ptr<TouchPoint>> touch_points() const;
  bool running() const;

  void start();
  void stop();
  bool poll_events(TouchPtrSet& new_touch_points,
                   TouchPtrSet& updated_touch_points,
                   TouchPtrSet& finished_touch_points);

 protected:
  void run();
  void update();

 protected:
  std::atomic<bool> _running;
  struct libevdev* _dev = nullptr;
  int _fd;

  std::string _path;
  std::string _name;

  unsigned int _num_slots;
  int _x_min, _x_max;
  int _y_min, _y_max;

  std::vector<double> _touch_positions_x;
  std::vector<double> _touch_positions_y;
  std::vector<std::shared_ptr<TouchPoint>> _touch_points;

  std::mutex _sync_mutex;
  std::set<std::shared_ptr<TouchPoint>> _new_touch_points;
  std::set<std::shared_ptr<TouchPoint>> _updated_touch_points;
  std::set<std::shared_ptr<TouchPoint>> _finished_touch_points;
  std::set<std::shared_ptr<TouchPoint>> _synced_new_touch_points;
  std::set<std::shared_ptr<TouchPoint>> _synced_updated_touch_points;
  std::set<std::shared_ptr<TouchPoint>> _synced_finished_touch_points;
};

}  // namespace wand