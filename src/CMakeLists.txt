
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
target_link_libraries(kcm_kalternatives KF6::ConfigWidgets KF6::CoreAddons KF6::I18n KF6::IconThemes KF6::KIOWidgets KF6::WidgetsAddons KF6::KCMUtils)
kcmutils_generate_desktop_file(kcm_kalternatives)

install(TARGETS kcm_kalternatives DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings)

