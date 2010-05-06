#pragma once

// CMake uses config.cmake.hh to generate config.hh within the build folder.
#ifndef CONFIG_HH
#define CONFIG_HH

#define PACKAGE "@CMAKE_PROJECT_NAME@"
#define SHARED_DATA_DIR "@SHARE_INSTALL@"

#cmakedefine USE_THREADS
#cmakedefine USE_NETWORK

#endif

