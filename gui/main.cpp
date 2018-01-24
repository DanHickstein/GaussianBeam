/* This file is part of the GaussianBeam project
   Copyright (C) 2007-2010 Jérôme Lodewyck <jerome dot lodewyck at normalesup.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "gui/GaussianBeamWidget.h"
#include "gui/GaussianBeamWindow.h"
#include "gui/Names.h"

#include <QApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QLibraryInfo>

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(GaussianBeam);

	QApplication app(argc, argv);

	QCoreApplication::setOrganizationName("GaussianBeam");
	QCoreApplication::setApplicationName("GaussianBeam");
	QCoreApplication::setApplicationVersion("0.5");

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF8"));
#endif

	QString locale = QLocale::system().name();
	// Load translations for the qt library
	QTranslator qtTranslator;
	qtTranslator.load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	app.installTranslator(&qtTranslator);
	// Load translations for GaussianBeam
	QTranslator translator;
	translator.load(":/po/GaussianBeam_" + locale);
	app.installTranslator(&translator);

	initNames(&app);

	QString file;
	if (argc > 1)
		file = argv[1];

	GaussianBeamWindow window(file);
	window.show();
	return app.exec();
}
