project(kalternatives)

# Search KDE installation
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${KDE4_INCLUDES})

# ### readd the distro selection
add_definitions(-DDEBIAN=1)

add_subdirectory(src)
