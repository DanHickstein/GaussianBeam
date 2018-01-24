######################################################################
# Automatically generated by qmake (2.01a) dim. mai 20 14:36:38 2007
######################################################################

# Uncomment to build the unit tests
# CONFIG += unittest

include(po/po.pri)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = gaussianbeam
DEPENDPATH += .
QT += xml xmlpatterns
QMAKE_CXXFLAGS += -pedantic -Wno-long-long -Wno-unused-local-typedefs -g
CONFIG += release warn_on stl qt
macx:CONFIG += x86 ppc                # Generate Universal Binary for Mac OS X
win32:RC_FILE = gui/GaussianBeam.rc   # Embed the application icon
CODECFORTR     = UTF-8
CODECFORSRC    = UTF-8

# Destinations
BIN_DIR  = $(CUSTOM_DESTDIR)/usr/bin
DATA_DIR = $(CUSTOM_DESTDIR)/usr/share/gaussianbeam

# Files to install
target.path = $$BIN_DIR
INSTALLS += target
images.path = $$DATA_DIR/images
images.files = gui/images/gaussianbeam*.png
INSTALLS += images

# Unit tests
unittest {
        QT += testlib
        SOURCES += test/test.cpp
        TARGET = gaussianbeamtest
}
!unittest{
        SOURCES += gui/main.cpp
}

# Input
# src
HEADERS += src/GaussianBeam.h src/Optics.h src/OpticsBench.h src/Statistics.h src/GaussianFit.h \
           src/Function.h src/OpticsFunction.h src/Cavity.h src/Utils.h src/lmmin.h src/Delegate.h
SOURCES += src/GaussianBeam.cpp src/Optics.cpp src/OpticsBench.cpp src/GaussianFit.cpp \
           src/Function.cpp src/OpticsFunction.cpp src/Cavity.cpp src/Utils.cpp src/lmmin.c
# gui
HEADERS += gui/GaussianBeamWidget.h gui/OpticsView.h gui/OpticsWidgets.h gui/GaussianBeamDelegate.h \
           gui/GaussianBeamModel.h gui/GaussianBeamWindow.h gui/Unit.h gui/Names.h
SOURCES += gui/GaussianBeamWidget.cpp gui/OpticsView.cpp gui/OpticsWidgets.cpp gui/GaussianBeamDelegate.cpp \
           gui/GaussianBeamModel.cpp gui/GaussianBeamWindow.cpp gui/Unit.cpp gui/Names.cpp \
           gui/GaussianBeamSave.cpp gui/GaussianBeamLoad.cpp
FORMS   += gui/GaussianBeamWidget.ui gui/GaussianBeamWindow.ui gui/OpticsViewProperties.ui
RESOURCES = gui/GaussianBeam.qrc