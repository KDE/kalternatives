# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

# these are the headers for your project
noinst_HEADERS = kalternatives.h altparser.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kalternatives.pot

KDE_ICON = kalternatives

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kalternatives

# the application source, library search path, and link libraries
kalternatives_SOURCES = main.cpp kalternatives.cpp altparser.cpp
kalternatives_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kalternatives_LDADD = $(LIB_KDEUI)

# this is where the desktop file will go 
shelldesktopdir   = $(kde_appsdir)/Utilities
shelldesktop_DATA = kalternatives.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir   = $(kde_datadir)/kalternatives
shellrc_DATA = kalternativesui.rc

