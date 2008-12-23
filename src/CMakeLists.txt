
# kcm

set(kcm_SRCS
  addalternatives.cpp
  alternativemodels.cpp
  altparser.cpp
  kalternatives.cpp
  slavemodel.cpp
  slavewidget.cpp
)
kde4_add_ui_files(kcm_SRCS
  addalternativesui.ui
  mainwindow.ui
  propertieswindow.ui
  slavewidget.ui
)
kde4_add_plugin(kcm_kalternatives ${kcm_SRCS})
target_link_libraries(kcm_kalternatives ${KDE4_KIO_LIBS})
install(TARGETS kcm_kalternatives DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kalternatives.desktop DESTINATION ${SERVICES_INSTALL_DIR})

