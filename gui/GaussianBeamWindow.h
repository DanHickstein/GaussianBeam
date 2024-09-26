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

#ifndef GAUSSIANBEAMWINDOWS_H
#define GAUSSIANBEAMWINDOW_H

#include "src/OpticsBench.h"
#include "ui_GaussianBeamWindow.h"

#include <QMainWindow>
#include <QXmlStreamWriter>
#include <QDomElement>

class OpticsView;
class OpticsScene;
class GaussianBeamWidget;
class GaussianBeamModel;
class CornerWidget;
class TablePropertySelector;
class QItemSelectionModel;
class QDoubleSpinBox;
class QTableView;

class GaussianBeamWindow : public QMainWindow, private Ui::GaussianBeamWindow, protected OpticsBenchEventListener
{
Q_OBJECT

public:
	GaussianBeamWindow(const QString& fileName = QString());

public:
	void showTargetBeam(bool visible = true);
	void openFile(const QString& path = QString());
	void saveFile(const QString& path = QString());
	OpticsBench* bench() { return m_bench; }

public slots:
	void updateWidget(const QModelIndex& topLeft, const QModelIndex& bottomRight);

protected slots:
	void on_action_New_triggered()                { newFile();                         }
	void on_action_Close_triggered()              { close();                           }
	void on_action_Open_triggered()               { openFile();                        }
	void on_action_Save_triggered()               { saveFile(m_currentFile);           }
	void on_action_SaveAs_triggered()             { saveFile();                        }
	void on_action_AddOptics_triggered()          { insertOptics(LensType);            }
	void on_action_RemoveOptics_triggered();
	void on_action_AddLens_triggered()            { insertOptics(LensType);            }
	void on_action_AddFlatMirror_triggered()      { insertOptics(FlatMirrorType);      }
	void on_action_AddCurvedMirror_triggered()    { insertOptics(CurvedMirrorType);    }
	void on_action_AddFlatInterface_triggered()   { insertOptics(FlatInterfaceType);   }
	void on_action_AddCurvedInterface_triggered() { insertOptics(CurvedInterfaceType); }
	void on_action_AddGenericABCD_triggered()     { insertOptics(GenericABCDType);     }
	void on_action_AddDielectricSlab_triggered()  { insertOptics(DielectricSlabType);  }
	void wavelengthSpinBox_valueChanged(double wavelength);
	void openRecentFile();

protected:
	virtual void onOpticsBenchSphericityChanged();
	virtual void onOpticsBenchWavelengthChanged();
	virtual void onOpticsBenchModified();

protected:
	virtual void showEvent(QShowEvent* event);
	virtual void closeEvent(QCloseEvent* event);

private:
	QWidget* createViewEnsemble(OpticsView* view);
	void newFile();
	void setCurrentFile(const QString& fileName);
	void updateRecentFileActions();
	void insertOptics(OpticsType opticsType);
	void readSettings();
	void writeSettings();

// Loading stuff that should logically be moved to OpticsBench, but depend on Qt.
// In addition, a GaussianBeam file contains view properties that do not belong to OpticsBench.
private:
	void convertFormat(QByteArray* data, const QString& xsltPath) const;
	bool parseFile(const QString& path = QString());
	void parseXml(const QDomElement& element);
	void parseBench(const QDomElement& element);
	void parseTargetBeam(const QDomElement& element);
	void parseBeam(const QDomElement& element, Beam& beam);
	void parseFit(const QDomElement& element);
	void parseOptics(const QDomElement& element, QMap<int, Optics*>& opticsList, QMap<int, int>& lockTree);
	void parseView(const QDomElement& element);
	bool writeFile(const QString& path = QString());
	void writeOrientedElement(QXmlStreamWriter& xmlWriter, QString name, QString data, Orientation orientation) const;
	void writeBench(QXmlStreamWriter& xmlWriter) const;
	void writeWaist(QXmlStreamWriter& xmlWriter, const Beam* beam, Orientation orientation) const;
	void writeBeam(QXmlStreamWriter& xmlWriter, const Beam* beam) const;
	void writeOptics(QXmlStreamWriter& xmlWriter, const Optics* optics) const;
	void writeView(QXmlStreamWriter& xmlWriter) const;
	// Compatibility functions
	void parseInputBeam11(const QDomElement& element, QList<QString>& lockTree);

private:
	QToolBar* m_fileToolBar;
	QMenu*    m_addOpticsMenu;
	QMenu*    m_recentFilesMenu;
	static const int m_maxRecentFiles = 5;
	QAction*  m_recentFileAction[m_maxRecentFiles];
	QDoubleSpinBox* m_wavelengthSpinBox;
	GaussianBeamWidget* m_widget;
	GaussianBeamModel* m_model;
	QItemSelectionModel* m_selectionModel;
	QTableView* m_table;
	TablePropertySelector* m_tableConfigWidget;
	CornerWidget* m_tableCornerWidget;
	OpticsScene* m_hOpticsScene;
	OpticsScene* m_vOpticsScene;
	OpticsView* m_hOpticsView;
	OpticsView* m_vOpticsView;
	QWidget* m_hOpticsViewEnsemble;
	QWidget* m_vOpticsViewEnsemble;

	QString m_currentFile;
};

#endif
