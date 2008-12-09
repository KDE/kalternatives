project(kalternatives)

# Search KDE installation
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${KDE4_INCLUDES})

string(TOLOWER "${KALTERNATIVES_DISTRO}" _distro)
set(ABOUT_SUBDIR ${_distro})
if (_distro STREQUAL "debian")
   set(DISTRO_DEBIAN TRUE)
elseif (_distro STREQUAL "fedora")
   set(DISTRO_FEDORA TRUE)
endif (_distro STREQUAL "debian")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config-kalternatives.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/src/config-kalternatives.h)

add_subdirectory(src)
add_subdirectory(pics)
