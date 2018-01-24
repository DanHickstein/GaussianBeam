cmake_minimum_required(VERSION 2.6)
project(GaussianBeam)

# Dependencies
set(QT_MIN_VERSION "4.5.0")
find_package(Qt4 COMPONENTS QtCore QtGui QtXml QtXmlPatterns REQUIRED)
include(${QT_USE_FILE})

# Platform options
if(APPLE)
	set(CMAKE_OSX_ARCHITECTURES i386 ppc)   # Cuild universal library
	set(CMAKE_OSX_DEPLOYMENT_TARGET 10.4)   # Minimum version required
endif (APPLE)

# Compiler options
set(CMAKE_INCLUDE_CURRENT_DIR ON)
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-pedantic -Wall -Wno-long-long")
endif(CMAKE_COMPILER_IS_GNUCXX)

# Sources
set(gaussianbeam_src_SRCS src/GaussianBeam.cpp src/Optics.cpp src/OpticsBench.cpp src/GaussianFit.cpp
                          src/Function.cpp src/OpticsFunction.cpp src/Cavity.cpp src/Utils.cpp src/lmmin.c)
set(gaussianbeam_gui_SRCS gui/GaussianBeamWidget.cpp gui/OpticsView.cpp gui/OpticsWidgets.cpp gui/GaussianBeamDelegate.cpp
                          gui/GaussianBeamModel.cpp gui/GaussianBeamWindow.cpp gui/Unit.cpp gui/Names.cpp
                          gui/GaussianBeamSave.cpp gui/GaussianBeamLoad.cpp gui/main.cpp)
qt4_wrap_ui(gaussianbeam_ui_SRCS gui/GaussianBeamWidget.ui gui/GaussianBeamWindow.ui gui/OpticsViewProperties.ui)
qt4_wrap_cpp(gaussianbeam_moc_SRCS gui/GaussianBeamDelegate.h gui/GaussianBeamDelegate.h gui/GaussianBeamModel.h
                                   gui/GaussianBeamWidget.h gui/GaussianBeamWindow.h gui/OpticsView.h gui/OpticsView.h gui/OpticsWidgets.h)
qt4_add_resources(gaussianbeam_rc_SRCS gui/GaussianBeam.qrc)
set(gaussianbeam_SRCS ${gaussianbeam_src_SRCS} ${gaussianbeam_gui_SRCS} ${gaussianbeam_ui_SRCS} ${gaussianbeam_moc_SRCS} ${gaussianbeam_rc_SRCS})

# Translations. They are generated in the binary directory and are moved to the po/ source directory
# so that they are accessible to the ressource file
qt4_create_translation(gaussianbeam_qm_SRCS gui po/GaussianBeam_fr.ts)
add_custom_target(translations COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/*.qm ${CMAKE_CURRENT_SOURCE_DIR}/po/ DEPENDS ${gaussianbeam_qm_SRCS})

# Embed the application icon in the Windows executable
if(MINGW)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/GaussianBeamIco.o COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/gui/GaussianBeam.rc -o ${CMAKE_CURRENT_BINARY_DIR}/GaussianBeamIco.o)
  set(gaussianbeam_SRCS ${gaussianbeam_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/GaussianBeamIco.o)
else(MINGW)
  set(gaussianbeam_SRCS ${gaussianbeam_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/gui/GaussianBeam.rc)
endif(MINGW)

# gaussianbeam executable
add_executable(gaussianbeam ${gaussianbeam_SRCS})
target_link_libraries(gaussianbeam ${QT_LIBRARIES})
add_dependencies(gaussianbeam translations)
add_custom_command(TARGET gaussianbeam POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_SOURCE_DIR}/po/*.qm)

# install files
install(TARGETS gaussianbeam DESTINATION bin)

# Packaging
set(CPACK_GENERATOR DEB RPM TGZ)
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 5)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_PACKAGE_CONTACT      "Jérôme Lodewyck (jerome dot lodewyck at normalesup dot org)")
set(CPACK_DESCRIPTION_SUMMARY  "GaussianBeam is a GUI software that simulated Gaussian laser beams")
# Debian package
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
set(CPACK_DEBIAN_PACKAGE_DEPENDS      "libqt4-core (>= 4.5), libqt4-gui (>= 4.5), libqt4-xml (>= 4.5), libqt4-xmlpatterns (>= 4.5)")
set(CPACK_DEBIAN_PACKAGE_SECTION      "science")
# RPM package

# Windows installer
if(WIN32 AND NOT UNIX)
  set(CPACK_PACKAGE_EXECUTABLES "gaussianbeam" "GaussianBeam")
endif()
# Mac bundle
set(CPACK_BUNDLE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/gui/images/gaussianbeam128.png)

include(CPack)
