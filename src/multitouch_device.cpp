#include <multitouch_device.hpp>

namespace wand {

MultitouchDevice::MultitouchDevice(const std::string& path) : _path{path}, _running{false} {
  _fd = open(_path.c_str(), O_RDONLY | O_NONBLOCK);
  int err = libevdev_new_from_fd(_fd, &_dev);
  if (err != 0) {
    auto error_msg = std::string{strerror(-err)};
    throw std::runtime_error("Failed to open device: " + error_msg);
  }

  _name = std::string{libevdev_get_name(_dev)};

  if (!(libevdev_has_event_type(_dev, EV_ABS) && libevdev_has_event_code(_dev, EV_ABS, ABS_MT_TRACKING_ID) &&
        libevdev_has_event_code(_dev, EV_ABS, ABS_MT_POSITION_X) &&
        libevdev_has_event_code(_dev, EV_ABS, ABS_MT_POSITION_Y))) {
    throw std::runtime_error("Device \"" + _name + "\" is not a multitouch device");
  }

  _num_slots = libevdev_get_num_slots(_dev);
  if (_num_slots < 1) {
    throw std::runtime_error("Device has invalid number of slots!");
  }
  _touch_positions_x = std::vector<double>(_num_slots, std::numeric_limits<double>::min());
  _touch_positions_y = std::vector<double>(_num_slots, std::numeric_limits<double>::min());
  _touch_points = std::vector<std::shared_ptr<TouchPoint>>{_num_slots};

  _x_min = libevdev_get_abs_minimum(_dev, ABS_MT_POSITION_X);
  _x_max = libevdev_get_abs_maximum(_dev, ABS_MT_POSITION_X);
  _y_min = libevdev_get_abs_minimum(_dev, ABS_MT_POSITION_Y);
  _y_max = libevdev_get_abs_maximum(_dev, ABS_MT_POSITION_Y);
}

MultitouchDevice::~MultitouchDevice() {
  if (_running) {
    stop();
  }
}

const std::string& MultitouchDevice::name() const {
  return _name;
}

int MultitouchDevice::num_slots() const {
  return _num_slots;
}

std::set<std::shared_ptr<TouchPoint>> MultitouchDevice::touch_points() const {
  std::set<std::shared_ptr<TouchPoint>> touch_points;
  for (auto& touch_point : _touch_points) {
    if (nullptr != touch_point) {
      touch_points.insert(touch_point);
    }
  }
  return touch_points;
}

void MultitouchDevice::start() {
  std::thread thread(&MultitouchDevice::run, this);
  thread.detach();
}

void MultitouchDevice::stop() {
  _running = false;

  // close the input device
  if (_dev != nullptr) {
    libevdev_free(_dev);
    _dev = nullptr;
  }

  // close the file device
  if (_fd > 0) {
    close(_fd);
    _fd = 0;
  }
}

bool MultitouchDevice::poll_events(TouchPtrSet& new_touch_points,
                                   TouchPtrSet& updated_touch_points,
                                   TouchPtrSet& finished_touch_points) {
  std::lock_guard<std::mutex> lock{_sync_mutex};

  // check if there were any events at all
  if (_synced_new_touch_points.empty() && _synced_updated_touch_points.empty() &&
      _synced_finished_touch_points.empty()) {
    return false;
  }

  new_touch_points.insert(_synced_new_touch_points.begin(), _synced_new_touch_points.end());
  _synced_new_touch_points.clear();
  updated_touch_points.insert(_synced_updated_touch_points.begin(), _synced_updated_touch_points.end());
  _synced_updated_touch_points.clear();
  finished_touch_points.insert(_synced_finished_touch_points.begin(), _synced_finished_touch_points.end());
  _synced_finished_touch_points.clear();

  return true;
}

void MultitouchDevice::run() {
  _running = true;
  while (_running) {
    update();
  }
}

void MultitouchDevice::update() {
  if (!libevdev_has_event_pending(_dev)) {
    return;
  }

  int rc = 0;
  while (rc >= 0) {
    struct input_event ev;
    rc = libevdev_next_event(_dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);

    if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
      if (ev.type == EV_ABS) {
        int slot = libevdev_get_current_slot(_dev);

        if (ev.code == ABS_MT_TRACKING_ID) {
          int id = ev.value;

          if (id >= 0) {
            // new touch
            auto touch = std::make_shared<TouchPoint>(id);
            _touch_points[slot] = touch;
            _new_touch_points.insert(touch);
          } else {
            // touch was lifted
            _touch_positions_x[slot] = std::numeric_limits<double>::min();
            _touch_positions_y[slot] = std::numeric_limits<double>::min();
            auto touch = _touch_points[slot];
            touch->finish();
            _finished_touch_points.insert(touch);
          }
        } else if (ev.code == ABS_MT_POSITION_X) {
          _touch_positions_x[slot] = (ev.value - _x_min) / double(_x_max - _x_min);
        } else if (ev.code == ABS_MT_POSITION_Y) {
          _touch_positions_y[slot] = (ev.value - _y_min) / double(_y_max - _y_min);
        }
      } else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
        // sync event
        for (std::size_t slot = 0; slot < _num_slots; ++slot) {
          auto touch_point = _touch_points[slot];
          if (touch_point && touch_point->active()) {
            touch_point->update(_touch_positions_x[slot], _touch_positions_y[slot]);
            _updated_touch_points.insert(touch_point);
          }
        }

        std::lock_guard<std::mutex> lock{_sync_mutex};
        _synced_new_touch_points.insert(_new_touch_points.begin(), _new_touch_points.end());
        _new_touch_points.clear();
        _synced_updated_touch_points.insert(_updated_touch_points.begin(), _updated_touch_points.end());
        _updated_touch_points.clear();
        _synced_finished_touch_points.insert(_finished_touch_points.begin(), _finished_touch_points.end());
        _finished_touch_points.clear();
      }
    }
  }
}

}  // namespace wand