#include <pybind11/pybind11.h>

#include <pybind11/chrono.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <string>

#include <multitouch_device.hpp>
#include <touch_point.hpp>

namespace py = pybind11;

using namespace wand;

PYBIND11_MODULE(wand, m) {
  m.doc() = "";

  py::class_<TouchPoint, std::shared_ptr<TouchPoint>>{m, "TouchPoint"}
      .def(py::init<int>(), py::arg("id"))
      .def("__repr__", [](const TouchPoint& touch) { return "<wand.TouchPoint " + std::to_string(touch.id()) + ">"; })
      .def_property_readonly("id", &TouchPoint::id)
      .def_property_readonly("active", &TouchPoint::active)
      .def_property_readonly("start_time", &TouchPoint::start_time)
      .def_property_readonly("update_time", &TouchPoint::update_time)
      .def_property_readonly("end_time", &TouchPoint::end_time)
      .def_property_readonly("duration", &TouchPoint::duration)
      .def_property_readonly("start_x", &TouchPoint::start_x)
      .def_property_readonly("start_y", &TouchPoint::start_y)
      .def_property_readonly("start_pos",
                             [](const TouchPoint& touch) {
                               std::array<double, 2> pos = {touch.start_x(), touch.start_y()};
                               return py::array_t<double>(2, pos.data());
                             })
      .def_property_readonly("x", &TouchPoint::x)
      .def_property_readonly("y", &TouchPoint::y)
      .def_property_readonly("pos",
                             [](const TouchPoint& touch) {
                               std::array<double, 2> pos = {touch.x(), touch.y()};
                               return py::array_t<double>(2, pos.data());
                             })
      .def_property_readonly(
          "direction",
          [](const TouchPoint& touch) {
            std::array<double, 2> direction = {touch.x() - touch.start_x(), touch.y() - touch.start_y()};
            return py::array_t<double>(2, direction.data());
          })
      .def_property_readonly("timestamps", &TouchPoint::timestamps)
      .def_property_readonly("x_positions", &TouchPoint::x_positions)
      .def_property_readonly("y_positions", &TouchPoint::y_positions);

  py::class_<MultitouchDevice, std::shared_ptr<MultitouchDevice>>{m, "MultitouchDevice"}
      .def(py::init<std::string>(), py::arg("path"))
      .def("__repr__",
           [](const MultitouchDevice& device) { return "<wand.MultitouchDevice \"" + device.name() + "\">"; })
      .def_property_readonly("name", &MultitouchDevice::name)
      .def_property_readonly("num_slots", &MultitouchDevice::num_slots)
      .def_property_readonly("touch_points", &MultitouchDevice::touch_points)
      .def_property_readonly("running", &MultitouchDevice::running)
      .def("start", &MultitouchDevice::start)
      .def("stop", &MultitouchDevice::stop)
      .def("poll_events", [](MultitouchDevice& dev) {
        TouchPtrSet new_touch_points, updated_touch_points, finished_touch_points;
        dev.poll_events(new_touch_points, updated_touch_points, finished_touch_points);
        return std::make_tuple(std::move(new_touch_points), std::move(updated_touch_points),
                               std::move(finished_touch_points));
      });
}
