cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6)

# Name of the executable
set(EXENAME "game")

# Avoid source tree pollution
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files already created:\nrm -rf CMakeCache.txt CMakeFiles")
endif()

# Add a sensible build type default and warning because empty means no optimization and no debug info.
if(NOT CMAKE_BUILD_TYPE)
	message("WARNING: CMAKE_BUILD_TYPE is not defined!\n         Defaulting to CMAKE_BUILD_TYPE=Release.\n         Use ccmake to set a proper value.")
	SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# Set default compile flags for GCC
if(CMAKE_COMPILER_IS_GNUCXX)
	message(STATUS "GCC detected, adding compile flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++98 -Wall -Wno-missing-field-initializers")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")

find_package(OpenGL)
include_directories(${OPENGL_INCLUDE_DIR})
list(APPEND LIBS ${OPENGL_LIBRARIES})

find_package(SDL)
include_directories(${SDL_INCLUDE_DIR})
list(APPEND LIBS ${SDL_LIBRARY})

find_package(Box2D)
include_directories(${BOX2D_INCLUDE_DIRS})
list(APPEND LIBS ${BOX2D_LIBRARIES})

find_package(Boost)
include_directories(${BOOST_INCLUDE_DIRS})

file(GLOB SOURCE_FILES "*.cc")
file(GLOB HEADER_FILES "*.hh")

set(SOURCES ${SOURCE_FILES} ${HEADER_FILES})

add_executable(${EXENAME} ${SOURCES})
target_link_libraries(${EXENAME} ${LIBS})


