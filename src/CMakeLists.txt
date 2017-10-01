
# kcm

set(kcm_SRCS
  addalternatives.cpp
  alternativemodels.cpp
  altparser.cpp
  kalternatives.cpp
  slavemodel.cpp
  slavewidget.cpp
)
ki18n_wrap_ui(kcm_SRCS
  addalternativesui.ui
  mainwindow.ui
  propertieswindow.ui
  slavewidget.ui
)
ecm_qt_declare_logging_category(kcm_SRCS
  HEADER kalternatives_debug.h
  IDENTIFIER KALT_LOG
  CATEGORY_NAME kalternatives
  DEFAULT_SEVERITY Warning
)
add_library(kcm_kalternatives MODULE ${kcm_SRCS})
target_link_libraries(kcm_kalternatives KF5::ConfigWidgets KF5::CoreAddons KF5::I18n KF5::IconThemes KF5::KIOWidgets KF5::KDELibs4Support KF5::WidgetsAddons)
install(TARGETS kcm_kalternatives DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES kalternatives.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

