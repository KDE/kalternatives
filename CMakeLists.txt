cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
project(kalternatives)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)

find_package(Qt5 CONFIG REQUIRED Core Gui Widgets)

find_package(KF5 REQUIRED
   ConfigWidgets
   CoreAddons
   I18n
   IconThemes
   KIO
   WidgetsAddons
)

string(TOLOWER "${KALTERNATIVES_DISTRO}" _distro)
set(ABOUT_SUBDIR ${_distro})
if (_distro STREQUAL "dpkg")
   set(DISTRO_DPKG TRUE)
elseif (_distro STREQUAL "rpm_2")
   set(DISTRO_RPM_2 TRUE)
elseif (_distro STREQUAL "rpm")
   set(DISTRO_RPM TRUE)
else ()
   message(FATAL_ERROR "A package manager was not set for KAlternatives. Please define KALTERNATIVES_DISTRO with its type (dpkg/rpm/rpm_2).")
endif ()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config-kalternatives.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/src/config-kalternatives.h)

add_subdirectory(src)
add_subdirectory(pics)
