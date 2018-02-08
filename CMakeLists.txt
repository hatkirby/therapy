cmake_minimum_required (VERSION 3.1)
project (Aromatherapy)

set(CMAKE_BUILD_TYPE Debug)

# Set directory to look for package helpers.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${Aromatherapy_SOURCE_DIR}/cmake")

# Get dependencies.

find_package(PkgConfig)
pkg_check_modules(GLFW REQUIRED glfw3)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(portaudio REQUIRED)
find_package(libsndfile REQUIRED)
find_package(libxml2 REQUIRED)

set(ALL_LIBS
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${GLFW_LIBRARIES}
  ${PORTAUDIO_LIBRARIES}
  ${LIBSNDFILE_LIBRARY}
  ${LIBXML2_LIBRARIES}
)

include_directories(
  ${LIBXML2_INCLUDE_DIR}
  ${GLFW_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  src
)

link_directories(
  ${GLFW_LIBRARY_DIRS}
)

add_executable(Aromatherapy
  src/main.cpp
  src/map.cpp
  src/renderer.cpp
  src/entity.cpp
  src/game.cpp
  src/muxer.cpp
  src/entityfactory.cpp
  src/world.cpp
  src/components/ai.cpp
  src/components/map_collision.cpp
  src/components/map_render.cpp
  src/components/physics_body.cpp
  src/components/player_physics.cpp
  src/components/player_sprite.cpp
  src/components/pondering.cpp
  src/components/simple_collider.cpp
  src/components/static_image.cpp
  src/components/user_movement.cpp
)

set_property(TARGET Aromatherapy PROPERTY CXX_STANDARD 11)
set_property(TARGET Aromatherapy PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(Aromatherapy ${ALL_LIBS})
