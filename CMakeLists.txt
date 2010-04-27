project("Tomaatti" CXX C)
cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6)

# Name of the executable
set(EXENAME ${CMAKE_PROJECT_NAME})
# Name of the directory
set(DIRNAME "tomaatti")

# Options
OPTION(USE_THREADS "Enable multi-threading support." ON)
OPTION(USE_NETWORK "Enable networking support." ON)

# Avoid source tree pollution
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files already created:\nrm -rf CMakeCache.txt CMakeFiles")
endif()

# Add a sensible build type default and warning because empty means no optimization and no debug info.
if (NOT CMAKE_BUILD_TYPE)
	message("WARNING: CMAKE_BUILD_TYPE is not defined!\n         Defaulting to CMAKE_BUILD_TYPE=RelWithDebInfo.\n         Use ccmake to set a proper value.")
	SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

if (WIN32)
	set(SHARE_INSTALL "." CACHE STRING "Data file install path. Must be a relative path (from CMAKE_INSTALL_PREFIX), with no trailing slash.")
else (WIN32)
	if(NOT SHARE_INSTALL)
		set(SHARE_INSTALL "share/games/${DIRNAME}" CACHE STRING "Data file install path. Must be a relative path (from CMAKE_INSTALL_PREFIX), with no trailing slash.")
	endif(NOT SHARE_INSTALL)
endif (WIN32)
mark_as_advanced(SHARE_INSTALL)

# Set default compile flags for GCC
if (CMAKE_COMPILER_IS_GNUCXX)
	message(STATUS "GCC detected, adding compile flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Libraries required by multi-threading
if (USE_THREADS)
	list(APPEND BOOST_COMPONENTS thread)
endif()

# Libraries required by networking
if (USE_NETWORK)
	if (NOT USE_THREADS)
		message(FATAL_ERROR "Networking requires multi-threading (USE_THREADS) to be enabled.")
	endif()
	find_package(ENet)
	include_directories(${ENet_INCLUDE_DIRS})
	list(APPEND LIBS ${ENet_LIBRARIES})
endif()

# Boost libraries
list(APPEND BOOST_COMPONENTS filesystem system)
find_package(Boost 1.36 REQUIRED COMPONENTS ${BOOST_COMPONENTS})
include_directories(${Boost_INCLUDE_DIRS})
list(APPEND LIBS ${Boost_LIBRARIES})

# Find all the libs that don't require extra parameters
foreach(lib Box2D Freetype SDL SOIL OpenGL)
	find_package(${lib} REQUIRED)
	include_directories(${${lib}_INCLUDE_DIRS})
	list(APPEND LIBS ${${lib}_LIBRARIES})
	add_definitions(${${lib}_DEFINITIONS})
endforeach(lib)


# Gather sources
file(GLOB SOURCE_FILES "*.cc")
file(GLOB HEADER_FILES "*.hh")

set(SOURCES ${SOURCE_FILES} ${HEADER_FILES})

# Generate config.hh
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.cmake.hh" "${CMAKE_CURRENT_BINARY_DIR}/config.hh" @ONLY)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Executable
add_executable(${EXENAME} ${SOURCES})
target_link_libraries(${EXENAME} ${LIBS})

# Installation
file(GLOB IMAGE_FILES "images/*.png")
file(GLOB CONF_FILES "*.conf")
set(DOC_FILES "docs/Readme.txt" "docs/Authors.txt" "docs/Compiling.txt")

install(FILES ${IMAGE_FILES} DESTINATION ${SHARE_INSTALL}/images)
install(FILES ${CONF_FILES} DESTINATION ${SHARE_INSTALL})
install(FILES ${DOC_FILES} DESTINATION ${SHARE_INSTALL}/doc)
install(TARGETS ${EXENAME} DESTINATION bin)

if (WIN32)
	file(GLOB DLL_FILES "bin/*.dll")
	install(FILES ${DLL_FILES} DESTINATION bin)
endif(WIN32)

