INCLUDES = $(all_includes)

noinst_HEADERS = kalternatives.h altparser.h altcontroller.h altitemelement.h treeitemelement.h addslaves.h addalternatives.h 

KDE_ICON = kalternatives

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kcm_kalternatives.la

kcm_kalternatives_la_SOURCES = addslavesui.ui propertieswindow.ui addalternativesui.ui mainwindow.ui altcontroller.cpp kalternatives.cpp altparser.cpp altitemelement.cpp treeitemelement.cpp addslaves.cpp addalternatives.cpp 

kcm_kalternatives_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO)
kcm_kalternatives_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined


kcm_kalternatives_la_METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kalternatives.pot
	
xdg_apps_DATA = kalternatives.desktop
