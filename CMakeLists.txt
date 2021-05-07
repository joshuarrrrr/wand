cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

project(wand)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(LIBEVDEV REQUIRED)

include_directories(${LIBEVDEV_INCLUDE_DIRS})
include_directories(${PYTHON_INCLUDE_DIR})
add_subdirectory(external/pybind11)
include_directories(external/pybind11/include)

include_directories(src)

pybind11_add_module(wand
    src/multitouch_device.cpp
    src/touch_point.cpp
    src/bindings.cpp
)
target_link_libraries(wand PRIVATE ${LIBEVDEV_LIBRARY})
SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
install(TARGETS wand DESTINATION ${CMAKE_SOURCE_DIR})