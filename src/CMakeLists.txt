
add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

# kcm

set(kcm_SRCS
  addslaves.cpp
  addalternatives.cpp
  altcontroller.cpp
  altitemelement.cpp
  altparser.cpp
  kalternatives.cpp
  treeitemelement.cpp
)
kde4_add_ui_files(kcm_SRCS
  addalternativesui.ui
  mainwindow.ui
)
kde4_add_ui3_files(kcm_SRCS
  addslavesui.ui
  propertieswindow.ui
)
kde4_add_plugin(kcm_kalternatives ${kcm_SRCS})
target_link_libraries(kcm_kalternatives ${KDE4_KDE3SUPPORT_LIBS})
install(TARGETS kcm_kalternatives DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kalternatives.desktop DESTINATION ${SERVICES_INSTALL_DIR})


# misc

kde4_install_icons(${ICON_INSTALL_DIR})


