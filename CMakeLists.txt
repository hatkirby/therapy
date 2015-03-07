cmake_minimum_required(VERSION 2.6)
project(Aromatherapy)

# Use our modified FindSDL2* modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${Aromatherapy_SOURCE_DIR}/cmake")
# Set an output directory for our binaries
set(BIN_DIR ${Aromatherapy_SOURCE_DIR}/bin)

# Bump up warning levels appropriately for clang, gcc & msvc
# Also set debug/optimization flags depending on the build type. IDE users choose this when
# selecting the build mode in their IDE
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -std=c++11")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
endif()

# Look up SDL2 and add the include directory to our include path
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)

set(ALL_LIBS
  ${OPENGL_LIBRARY}
  ${GLEW_LIBRARY}
  ${GLFW_LIBRARY}
)

# include_directories(${SDL2_INCLUDE_DIR})
set(CMAKE_BUILD_TYPE Debug)
add_executable(Aromatherapy src/main.cpp src/map.cpp src/renderer.cpp src/world.cpp src/entity.cpp src/components.cpp src/game.cpp)
target_link_libraries(Aromatherapy ${ALL_LIBS})
install(TARGETS Aromatherapy RUNTIME DESTINATION ${BIN_DIR})
