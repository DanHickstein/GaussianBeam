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

#ifndef OPTICSVIEW_H
#define OPTICSVIEW_H

#include "src/GaussianBeam.h"
#include "src/Optics.h"
#include "src/OpticsBench.h"

#include <QPoint>
#include <QPainterPath>

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QStatusBar>

class QAbstractItemModel;
class QComboBox;

class OpticsItem;
class BeamItem;
class RullerSlider;
class OpticsViewProperties;
class StatusWidget;

class OpticsScene : public QGraphicsScene, public OpticsBenchEventListener
{
Q_OBJECT

public:
	OpticsScene(OpticsBench* bench, Orientation orientation = Horizontal, QObject* parent = 0);

public:
	OpticsScene* otherScene() const { return m_otherScene; }
	void setOtherScene(OpticsScene* otherScene) { m_otherScene = otherScene; }
	Orientation orientation() const { return m_orientation; }
	void showTargetBeam(bool show = true);
	bool targetBeamVisible() const;
	double beamScale() const { return m_beamScale; }
	void setBeamScale(double beamScale);
	double opticsHeight() const { return m_opticsHeight; }
	void setOpticsHeight(double opticsHeight);
	bool scenesLocked() const { return m_scenesLocked; }
	void setScenesLocked(bool scenesLocked);

protected:
	virtual void onOpticsBenchDataChanged(int startOptics, int endOptics);
	virtual void onOpticsBenchTargetBeamChanged();
	virtual void onOpticsBenchBoundariesChanged();
	virtual void onOpticsBenchOpticsAdded(int index);
	virtual void onOpticsBenchOpticsRemoved(int index, int count);
	virtual void onOpticsBenchFitDataChanged(int index);
	virtual void onOpticsBenchFitsRemoved(int index, int count);
	virtual void onOpticsBenchSphericityChanged();
	virtual void onOpticsBenchDimensionalityChanged();

private:
	void addFitPoint(double position, double radius, QRgb color);

private:
	OpticsScene* m_otherScene;
	Orientation m_orientation;
	double m_beamScale;
	double m_opticsHeight;
	bool m_scenesLocked;

	QList<BeamItem*> m_beamItems;
	BeamItem* m_targetBeamItem;
	BeamItem* m_cavityBeamItem;
	QList<QGraphicsEllipseItem*> m_fitItems;
};

class OpticsView : public QGraphicsView
{
Q_OBJECT

public:
	OpticsView(QGraphicsScene* scene, OpticsBench* bench);

public:
	void setStatusWidget(StatusWidget* statusWidget) { m_statusWidget = statusWidget; }
	double horizontalRange() const { return m_horizontalRange; }
	void setHorizontalRange(double horizontalRange);
	QPointF origin() const { return m_origin; }
	void setOrigin(QPointF origin);
	/// Zoom the view by @p factor, leaving @p center invariant. @todo: implement a 2D version of this function
	void zoom(double center, double factor);
	/// Zoom the view by @p factor, leaving the center of the view invariant
	void zoom(double factor);
	void showProperties(bool show = true);
	bool propertiesVisible() const;
	OpticsViewProperties* propertiesWidget() { return m_opticsViewProperties; }
	void showFullBench();

signals:
	void rangeChanged();

/// Inherited protected functions
protected:
	virtual void resizeEvent(QResizeEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void wheelEvent(QWheelEvent* e);
	virtual void drawBackground(QPainter* painter, const QRectF& rect);

private:
	void adjustRange();

private:
	OpticsBench* m_bench;
	OpticsViewProperties* m_opticsViewProperties;
	StatusWidget* m_statusWidget;
	double m_horizontalRange;
	QPointF m_origin;

friend class OpticsScene;
};

/// @todo don't forget prepareGeometryChange()
class OpticsItem : public QGraphicsItem
{
public:
	OpticsItem(const Optics* optics, OpticsBench* bench);

/// Inherited public functions
public:
	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

/// Inherited protected functions
protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant& value);

public:
	void setUpdate(bool update) { m_update = update; }
	const Optics* optics() const { return m_optics; }
	void setOptics(const Optics* optics) { m_optics = optics; }
	void prepareHeightChange() { prepareGeometryChange(); }
	void updateNameLabel();

private:
	const Optics* m_optics;
	bool m_update;
	OpticsBench* m_bench;
	QGraphicsSimpleTextItem* m_label;
};

class BeamItem : public QGraphicsItem
{
public:
	BeamItem(const Beam* beam, const Beam* previousBeam = 0, const Beam* nextBeam = 0);

/// Inherited public functions
public:
	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

public:
	void updateTransform();
	const Beam* beam() const { return m_beam; }
	void setPlainStyle(bool style = true) { m_style = style; }
	bool auxiliary() const { return m_auxiliary; }
	void setAuxiliary(bool auxiliary) { m_auxiliary = auxiliary; }
	void setPreviousBeam(const Beam* previousBeam) { m_previousBeam = previousBeam; }
	void setNextBeam(const Beam* nextBeam) { m_nextBeam = nextBeam; }

private:
	void drawUpperBeamSegment(double start, double stop, double pixel, int nStep, QPolygonF& polygon) const;
	void drawLowerBeamSegment(double start, double stop, double pixel, int nStep, QPolygonF& polygon) const;

private:
	const Beam* m_beam;
	const Beam* m_previousBeam;
	const Beam* m_nextBeam;

	bool m_drawText;
	bool m_style;
	bool m_auxiliary;

	double m_startLowerCache, m_startUpperCache;
	double m_stopLowerCache, m_stopUpperCache;
	QRectF m_boundingRectCache;
	Orientation m_orientationCache;
};

#endif
