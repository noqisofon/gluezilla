AM_CXXFLAGS = @autoconf_default_CXXFLAGS@

#bin_PROGRAMS = demo

lib_LTLIBRARIES = libgluezilla.la

INCLUDES = $(GLUEZILLA_CFLAGS) -DXUL_VERSION=$(XUL_VERSION)

libgluezilla_la_SOURCES = 	\
	interfaces.h \
	SecurityWarningsDialogs.cpp	\
	SecurityWarningsDialogs.h	\
	PromptService.cpp	\
	PromptService.h	\
	LocationProvider.cpp	\
	LocationProvider.h	\
	BrowserWindow.cpp	\
	BrowserWindow.h	\
	Widget.cpp	\
	Widget.h	\
	EventListener.cpp	\
	EventListener.h	\
	gtkWidget.cpp	\
	gtkWidget.h	\
	gluezilla.cpp	\
	gluezilla.h   \
	FastDelegate.h

if XUL_2
libgluezilla_la_SOURCES += \
	gluezilla_nsIScriptContext.h \
	gluezilla_nsIScriptGlobalObject.h
endif

libgluezilla_la_LIBADD = $(GLUEZILLA_LIBS)

EXTRA_DIST = FastDelegate.h windows.mak *.manifest

debug:
	LD_LIBRARY_PATH=.libs gdb 

valgrind:
	LD_LIBRARY_PATH=.libs valgrind
