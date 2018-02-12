cmake_minimum_required (VERSION 3.1)
project (AromatherapyMapEditor)

# Set directory to look for package helpers.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${AromatherapyMapEditor_SOURCE_DIR}/cmake")

find_package(wxWidgets REQUIRED core base)
include(${wxWidgets_USE_FILE})

find_package(libxml2 REQUIRED)

set(ALL_LIBS
  ${wxWidgets_LIBRARIES}
  ${LIBXML2_LIBRARIES}
)

include_directories(
  ${LIBXML2_INCLUDE_DIR}
  src
)

set(CMAKE_BUILD_TYPE Debug)
add_executable(AromatherapyMapEditor
  src/main.cpp
  src/map.cpp
  src/frame.cpp
  src/widget.cpp
  src/tile_widget.cpp
  src/object.cpp
  src/world.cpp
  src/undo.cpp
  src/mapselect_combo.cpp
)

set_property(TARGET AromatherapyMapEditor PROPERTY CXX_STANDARD 11)
set_property(TARGET AromatherapyMapEditor PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(AromatherapyMapEditor ${ALL_LIBS})
