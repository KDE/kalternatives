cmake_minimum_required(VERSION 2.8)
project(kalternatives)

# Search KDE installation
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${KDE4_INCLUDES})

string(TOLOWER "${KALTERNATIVES_DISTRO}" _distro)
set(ABOUT_SUBDIR ${_distro})
if (_distro STREQUAL "dpkg")
   set(DISTRO_DPKG TRUE)
elseif (_distro STREQUAL "rpm_2")
   set(DISTRO_RPM_2 TRUE)
elseif (_distro STREQUAL "rpm")
   set(DISTRO_RPM TRUE)
else(_distro STREQUAL "dpkg")
   message(FATAL_ERROR "A package manager was not set for KAlternatives. Please define KALTERNATIVES_DISTRO with its type (dpkg/rpm/rpm_2).")
endif (_distro STREQUAL "dpkg")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config-kalternatives.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/src/config-kalternatives.h)

add_subdirectory(src)
add_subdirectory(pics)
