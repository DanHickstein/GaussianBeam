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

#include "gui/OpticsView.h"
#include "gui/OpticsWidgets.h"
#include "gui/GaussianBeamModel.h"
#include "gui/Unit.h"
#include "src/GaussianBeam.h"
#include "src/Utils.h"
#include "src/OpticsBench.h"
#include "src/GaussianFit.h"

#include <QtGui>
#include <QtDebug>
#include <QtGlobal>

#include <cmath>

QColor wavelengthColor(double wavelength)
{
	wavelength *= 1e9;

	QColor color = Qt::black;

	// Wavelength to rgb conversion
	if (wavelength < 380.)
		color = QColor::fromRgbF(1., 0., 1.);
	else if (wavelength < 440.)
		color = QColor::fromRgbF((440.-wavelength)/60., 0., 1.);
	else if (wavelength < 490.)
		color = QColor::fromRgbF(0., (wavelength-440.)/50., 1.);
	else if (wavelength < 510.)
		color = QColor::fromRgbF(0., 1. , (510.-wavelength)/40.);
	else if (wavelength < 580.)
		color = QColor::fromRgbF((wavelength-510.)/70., 1., 0.);
	else if (wavelength < 645.)
		color = QColor::fromRgbF(1., (645.-wavelength)/85., 0.);
	else
		color = QColor::fromRgbF(1., 0., 0.);

	// Attenuation
	double attenuation = 1.;
	double minAttenuation = 0.37;
	if (wavelength > 700.)
		attenuation = qMax(minAttenuation, .3 + .7*(780.-wavelength)/80.);
	else if (wavelength < 420.)
		attenuation = qMax(minAttenuation, .3 + .7*(wavelength-380.)/40.);

	// Gamma
	double gamma = 0.8;
	color.setRedF  (pow(color.redF  ()*attenuation, gamma));
	color.setGreenF(pow(color.greenF()*attenuation, gamma));
	color.setBlueF (pow(color.blueF ()*attenuation, gamma));

	return color;
}

/////////////////////////////////////////////////
// OpticsScene class

OpticsScene::OpticsScene(OpticsBench* bench, Orientation orientation, QObject* parent)
	: QGraphicsScene(parent)
{
	m_otherScene = 0;
	m_orientation = orientation;
	m_beamScale = 100.;
	m_opticsHeight = 0.06;
	m_scenesLocked = true;

	// Bench connections
	m_bench = bench;
	m_bench->registerEventListener(this);

	setItemIndexMethod(QGraphicsScene::NoIndex);

	m_targetBeamItem = new BeamItem(m_bench->targetBeam());
	m_targetBeamItem->setPlainStyle(false);
	m_targetBeamItem->setAuxiliary(true);
	m_targetBeamItem->setPos(0., 0.);
	addItem(m_targetBeamItem);

	m_cavityBeamItem = new BeamItem(m_bench->cavity().eigenBeam(m_bench->wavelength(), 0));
	m_cavityBeamItem->setPlainStyle(false);
	m_cavityBeamItem->setAuxiliary(true);
	m_cavityBeamItem->setPos(0., 0.);
	m_cavityBeamItem->setVisible(false);
//	addItem(m_cavityBeamItem);

	// Sync with bench
	for (int i = 0; i < m_bench->nOptics(); i++)
		onOpticsBenchOpticsAdded(i);
	onOpticsBenchBoundariesChanged();
}

void OpticsScene::setBeamScale(double beamScale)
{
	if (beamScale == m_beamScale)
		return;

	if (beamScale > 0.)
		m_beamScale = beamScale;

	foreach (QGraphicsView* view, views())
	{
		dynamic_cast<OpticsView*>(view)->propertiesWidget()->setBeamScale(m_beamScale);
		dynamic_cast<OpticsView*>(view)->adjustRange();
	}

	foreach (BeamItem* item, m_beamItems)
		item->updateTransform();

	m_targetBeamItem->updateTransform();
	m_cavityBeamItem->updateTransform();

	if (m_scenesLocked && m_otherScene)
		m_otherScene->setBeamScale(m_beamScale);

	// Rescale fit points
	onOpticsBenchFitDataChanged(0);
}

void OpticsScene::setOpticsHeight(double opticsHeight)
{
	if (opticsHeight == m_opticsHeight)
		return;

	foreach (QGraphicsItem* item, items())
		if (OpticsItem* opticsItem = dynamic_cast<OpticsItem*>(item))
			opticsItem->prepareHeightChange();

	if (opticsHeight > 0.)
		m_opticsHeight = opticsHeight;

	foreach (QGraphicsView* view, views())
		dynamic_cast<OpticsView*>(view)->propertiesWidget()->setOpticsHeight(m_opticsHeight);

	foreach (QGraphicsItem* item, items())
		if (OpticsItem* opticsItem = dynamic_cast<OpticsItem*>(item))
			opticsItem->updateNameLabel();

	if (m_scenesLocked && m_otherScene)
		m_otherScene->setOpticsHeight(m_opticsHeight);
}

void OpticsScene::setScenesLocked(bool scenesLocked)
{
	if (scenesLocked == m_scenesLocked)
		return;

	m_scenesLocked = scenesLocked;

	foreach (QGraphicsView* view, views())
		dynamic_cast<OpticsView*>(view)->propertiesWidget()->setLock(scenesLocked);

	if (m_otherScene)
		m_otherScene->setScenesLocked(scenesLocked);
}

void OpticsScene::showTargetBeam(bool show)
{
	m_targetBeamItem->updateTransform();
	m_targetBeamItem->setVisible(show);
}

bool OpticsScene::targetBeamVisible() const
{
	return m_targetBeamItem->isVisible();
}

void OpticsScene::onOpticsBenchDataChanged(int startOptics, int endOptics)
{
	foreach (QGraphicsItem* graphicsItem, items())
	{
		if (OpticsItem* opticsItem = dynamic_cast<OpticsItem*>(graphicsItem))
		{
			int opticsIndex = m_bench->opticsIndex(opticsItem->optics());
			if ((opticsIndex >= startOptics) && (opticsIndex <= endOptics))
			{
				opticsItem->setUpdate(false);
				const Beam* axis = m_bench->axis(opticsIndex);
				Utils::Point coord = axis->absoluteCoordinates(opticsItem->optics()->position());
				opticsItem->setRotation(-(axis->angle() + opticsItem->optics()->angle())*180./M_PI);
				opticsItem->setPos(coord.x(), -coord.y());
				opticsItem->updateNameLabel();
				opticsItem->setUpdate(true);
			}
		}
	}

	for (int i = qMax(0, startOptics-1); i <= endOptics; i++)
		m_beamItems[i]->updateTransform();
}

void OpticsScene::onOpticsBenchTargetBeamChanged()
{
	m_targetBeamItem->updateTransform();
	m_targetBeamItem->update();
}

void OpticsScene::onOpticsBenchBoundariesChanged()
{
	Utils::Rect benchRect = m_bench->boundary();

	setSceneRect(QRectF(QPointF(benchRect.x1(), -benchRect.y2()), QPointF(benchRect.x2(), -benchRect.y1())));

	m_targetBeamItem->updateTransform();
	m_cavityBeamItem->updateTransform();

	if ((m_beamItems.size() != 0) && (m_beamItems.size() == m_bench->nOptics()))
	{
		m_beamItems[0]->updateTransform();
		m_beamItems[m_bench->nOptics()-1]->updateTransform();
	}

	foreach (QGraphicsView* view, views())
		dynamic_cast<OpticsView*>(view)->adjustRange();
}

void OpticsScene::onOpticsBenchOpticsAdded(int index)
{
	OpticsItem* opticsItem = new OpticsItem(m_bench->optics(index), m_bench);
	addItem(opticsItem);

	const Beam* beam = m_bench->beam(index);
	BeamItem* beamItem = new BeamItem(beam,                                                           // Added beam
									  (index > 0) ? m_bench->beam(index-1) : 0,                       // Previous beam
									  (index < m_bench->nOptics() - 1) ? m_bench->beam(index+1) : 0); // Next beam
	m_beamItems.insert(index, beamItem);
	if (index > 0)
		m_beamItems[index-1]->setNextBeam(beam);
	else if  (index < m_bench->nOptics() - 1)
		m_beamItems[index+1]->setPreviousBeam(beam);

	addItem(beamItem);
	beamItem->updateTransform();
}

void OpticsScene::onOpticsBenchOpticsRemoved(int index, int count)
{
	foreach (QGraphicsItem* graphicsItem, items())
		if (OpticsItem* opticsItem = dynamic_cast<OpticsItem*>(graphicsItem))
			if (m_bench->opticsIndex(opticsItem->optics()) == -1)
				removeItem(graphicsItem);

	for (int i = index + count - 1; i >= index; i--)
		removeItem(m_beamItems.takeAt(i));

	// Update neighbour beams
	if (index > 0)
		m_beamItems[index-1]->setNextBeam(0);
	if (index < m_bench->nOptics())
		m_beamItems[index]->setPreviousBeam(0);
	if ((index > 0) && (index < m_bench->nOptics()))
	{
		m_beamItems[index-1]->setNextBeam(m_bench->beam(index));
		m_beamItems[index]->setPreviousBeam(m_bench->beam(index-1));
	}
}

void OpticsScene::addFitPoint(double position, double radius, QRgb color)
{
	QGraphicsEllipseItem* fitItem = new QGraphicsEllipseItem(-2., -2., 4., 4.);
	fitItem->setFlags(fitItem->flags() | QGraphicsItem::ItemIgnoresTransformations);
	fitItem->setPen(QPen(color));
	fitItem->setBrush(QBrush(color));
	fitItem->setPos(position, radius*m_beamScale);
	fitItem->setZValue(2.);
	m_fitItems.push_back(fitItem);
	addItem(fitItem);
}

void OpticsScene::onOpticsBenchFitsRemoved(int index, int count)
{
	Q_UNUSED(count);
	/// @todo with more houskeeping in OpticsScene::OpticsBenchFitDataChanged this will not work any more
	/// @bug what 'index' has to do there ?
	onOpticsBenchFitDataChanged(index);
}

void OpticsScene::onOpticsBenchFitDataChanged(int index)
{
	Q_UNUSED(index);

	/// @todo with a little bit of housekeeping, we could avoid recontructing all the items each time...
	while (!m_fitItems.isEmpty())
	{
		removeItem(m_fitItems.last());
		m_fitItems.removeLast();
	}

	for (int index = 0; index < m_bench->nFit(); index++)
	{
		Fit* fit = m_bench->fit(index);
		const Orientation so = orientation();
		const Orientation fo = fit->orientation();
		if (!(((so == Horizontal) && (fo == Vertical)) || ((so == Vertical) && (fo == Horizontal))))
			for (int i = 0; i < fit->size(); i++)
				if (fit->value(i, so) != 0.)
				{
					addFitPoint(fit->position(i), -fit->radius(i, so), fit->color());
					addFitPoint(fit->position(i),  fit->radius(i, so), fit->color());
				}
	}
}

void OpticsScene::onOpticsBenchSphericityChanged()
{
	foreach (QGraphicsView* view, views())
		dynamic_cast<OpticsView*>(view)->propertiesWidget()->setLockVisible(!m_bench->isSpherical());
}

void OpticsScene::onOpticsBenchDimensionalityChanged()
{
	foreach (QGraphicsView* view, views())
		dynamic_cast<OpticsView*>(view)->adjustRange();
}

/////////////////////////////////////////////////
// OpticsView class

OpticsView::OpticsView(QGraphicsScene* scene, OpticsBench* bench)
	: QGraphicsView(scene)
	, m_bench(bench)
{
	setRenderHint(QPainter::Antialiasing);
	setBackgroundBrush(Qt::white);
	m_statusWidget = 0;

	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	m_opticsViewProperties = new OpticsViewProperties(this);
	m_opticsViewProperties->setLock(dynamic_cast<OpticsScene*>(scene)->scenesLocked());
	m_opticsViewProperties->setBeamScale(dynamic_cast<OpticsScene*>(scene)->beamScale());
	m_opticsViewProperties->setOpticsHeight(dynamic_cast<OpticsScene*>(scene)->opticsHeight());
	m_opticsViewProperties->hide();

	setResizeAnchor(QGraphicsView::AnchorViewCenter);

	setHorizontalRange(0.6);
	setOrigin(QPointF(-0.1, 0.));
}

void OpticsView::adjustRange()
{
	if (horizontalRange() == 0.)
		return;

	// Set the view rect exactly to the desired view to avoid scrollbar effects
	/// @TODO to be done for vertical
	setSceneRect(origin().x(), scene()->sceneRect().top(), horizontalRange(), scene()->height());
	// Transform the view so that it displays the view rect
	fitInView(sceneRect());
	// Change the vertical scale to have an orthonormal representation
	double vScale = matrix().m11()/matrix().m22();
	scale(1., vScale);

	emit(rangeChanged());
}

void OpticsView::showProperties(bool show)
{
	m_opticsViewProperties->setVisible(show);
}

bool OpticsView::propertiesVisible() const
{
	return m_opticsViewProperties->isVisible();
}

void OpticsView::setOrigin(QPointF origin)
{
	if (origin == m_origin)
		return;

	m_origin = origin;
	m_opticsViewProperties->setOrigin(m_origin);
	adjustRange();

	OpticsScene* opticsScene = dynamic_cast<OpticsScene*>(scene());
	OpticsScene* otherScene = opticsScene->otherScene();
	if (otherScene && opticsScene->scenesLocked())
	{
		foreach (QGraphicsView* view, otherScene->views())
			dynamic_cast<OpticsView*>(view)->setOrigin(OpticsView::origin());
	}
}

void OpticsView::setHorizontalRange(double horizontalRange)
{
	if ((horizontalRange == m_horizontalRange) || (horizontalRange == 0.))
		return;

	m_horizontalRange = horizontalRange;
	m_opticsViewProperties->setHorizontalRange(m_horizontalRange);
	adjustRange();

	OpticsScene* opticsScene = dynamic_cast<OpticsScene*>(scene());
	OpticsScene* otherScene = opticsScene->otherScene();
	if (otherScene && opticsScene->scenesLocked())
	{
		foreach (QGraphicsView* view, otherScene->views())
			dynamic_cast<OpticsView*>(view)->setHorizontalRange(OpticsView::horizontalRange());
	}
}

void OpticsView::zoom(double factor)
{
	zoom(origin().x() + 0.5*horizontalRange(), factor);
}

void OpticsView::zoom(double center, double factor)
{
	setOrigin(factor*origin() + QPointF((1. - factor)*center, 0.));
	setHorizontalRange(factor*horizontalRange());
//	setOrigin(origin() + QPointF(0.5*(1. - factor)*horizontalRange(), 0.));
//	setHorizontalRange(factor*horizontalRange());
}

void OpticsView::showFullBench()
{
	/// @todo make it work for 2D benches
	setHorizontalRange(scene()->sceneRect().width());
	setOrigin((scene()->sceneRect().topLeft() + scene()->sceneRect().bottomLeft())/2.);
}

void OpticsView::resizeEvent(QResizeEvent* event)
{
	QGraphicsView::resizeEvent(event);
	adjustRange();
	m_opticsViewProperties->move(viewport()->width() - m_opticsViewProperties->width(), viewport()->height() - m_opticsViewProperties->height());
}

void OpticsView::mouseMoveEvent(QMouseEvent* event)
{
	if (m_statusWidget)
	{
		QPointF position = mapToScene(event->pos());
		Utils::Point pos(position.x(), -position.y());
		std::pair<Beam*, double> p = m_bench->closestPosition(pos);

		m_statusWidget->showBeamInfo(p.first, p.second, dynamic_cast<OpticsScene*>(scene())->orientation());
	}

	QGraphicsView::mouseMoveEvent(event);
}

void OpticsView::wheelEvent(QWheelEvent* e)
{
	Q_UNUSED(e);
}

void OpticsView::drawBackground(QPainter* painter, const QRectF& rect)
{
	Q_UNUSED(painter);
	Q_UNUSED(rect);

	QBrush backgroundBrush(QColor(255,157,132,135));
	painter->fillRect(rect, backgroundBrush);
	painter->fillRect(scene()->sceneRect(), QBrush(Qt::white));

	QPen opticalAxisPen(Qt::darkGray, 0, Qt::DashDotLine, Qt::FlatCap, Qt::RoundJoin);
	painter->setPen(opticalAxisPen);
	painter->drawLine(QPointF(rect.left(), 0.), QPointF(rect.right(), 0.));
	painter->drawLine(QPointF(0., rect.top()),  QPointF(0., rect.bottom()));

	// Geometrical optics
/*
	QPolygonF polygon;


	double x = m_bench->optics(0)->image(Beam(), Beam()).waistPosition();
	double r = 0.;
	double t = m_bench->optics(0)->image(Beam(), Beam()).waist()*1000.;

	polygon << QPointF(x, r);

	for (int i = 0; i < m_bench->nOptics(); i++)
		if (m_bench->optics(i)->isABCD())
		{
			const ABCD* optics = dynamic_cast<const ABCD*>(m_bench->optics(i));
			double pos = optics->position();
			// Apply freespace
			r += t*(pos - x);
			// Apply ABCD
			double rp = optics->A(Spherical)*r + optics->B(Spherical)*t;
			double tp = optics->C(Spherical)*r + optics->D(Spherical)*t;
			r = rp; t = tp;
			// New point
			polygon << QPointF(pos, r);
			x = pos;
		}

	for (int i = polygon.size() - 1; i >= 0; i--)
		polygon << QPointF(polygon[i].x(), -polygon[i].y());

	QPen geometricalPen(Qt::red, 0);
	painter->setPen(geometricalPen);
	painter->setBrush(QBrush());
	painter->drawPolygon(polygon);
*/

/*	/// @todo if drawing a grid and rullers, set background cache
	const double pixel = scene()->width()/width();

	QBrush backgroundBrush(Qt::white);
	painter->fillRect(rect, backgroundBrush);

	QPen opticalAxisPen(Qt::lightGray, 2.*pixel, Qt::DashDotLine, Qt::FlatCap, Qt::RoundJoin);
	painter->setPen(opticalAxisPen);
	/// @todo replace scene()->sceneRect().left() by rect.left() once dashOffset works
	painter->drawLine(QPointF(scene()->sceneRect().left(), 0.), QPointF(rect.right(), 0.));


	QPen verticalAxisPen(Qt::lightGray, pixel, Qt::DotLine, Qt::FlatCap, Qt::RoundJoin);
	/// @todo idem
	painter->setPen(verticalAxisPen);
	painter->drawLine(QPointF(0., scene()->sceneRect().top()), QPointF(0., rect.bottom()));*/
}

/////////////////////////////////////////////////
// OpticsItem class

OpticsItem::OpticsItem(const Optics* optics, OpticsBench* bench)
	: QGraphicsItem()
	, m_optics(optics)
	, m_bench(bench)
{
	if (m_optics->type() != CreateBeamType)
	{
		setCursor(Qt::OpenHandCursor);
		setFlag(QGraphicsItem::ItemIsMovable);
		setFlag(QGraphicsItem::ItemSendsGeometryChanges);
		setZValue(1);
	}
	else
		setZValue(-1);

	m_label = new QGraphicsSimpleTextItem(this);

	m_update = true;
}

QRectF OpticsItem::boundingRect() const
{
	QRectF bounding;

	OpticsScene* opticsScene = dynamic_cast<OpticsScene*>(scene());
	if (!opticsScene)
	{
		qDebug() << "OpticsItem::boundingRect() without scene";
		return QRectF();
	}

	const double h = 0.5*opticsScene->opticsHeight();

	if (m_optics->width() == 0.)
		bounding = QRectF(QPointF(-0.15*h, -h), QSizeF(0.30*h, 2.*h));
	else
		bounding = QRectF(QPointF(0., -h), QSizeF(m_optics->width(), 2.*h));

	return bounding;
}

QVariant OpticsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
	if ((change == ItemPositionChange && scene()) && m_update)
	{
		QPointF newPos = value.toPointF();
		Utils::Point benchPosition(newPos.x(), -newPos.y());
/*		std::pair<Beam*, double> p = m_bench->closestPosition(benchPosition);

		// Propose the new position
		m_bench->setOpticsPosition(m_bench->opticsIndex(m_optics), p.second);*/

		int index = m_bench->opticsIndex(m_optics);
		if (index == 0)
			return pos();

		Utils::Point beamCoord = m_bench->beam(index-1)->beamCoordinates(benchPosition);
		m_bench->setOpticsPosition(m_bench->opticsIndex(m_optics), beamCoord.x());

		return pos();
	}

	return QGraphicsItem::itemChange(change, value);
}

void OpticsItem::updateNameLabel()
{
	if (m_optics->type() == CreateBeamType)
	{
		m_label->setText("");
		return;
	}

	QRectF rect = boundingRect();
	double pointSize = m_label->font().pointSizeF();
	double scale = rect.height()*0.1/pointSize;
	m_label->setText(QString::fromUtf8(m_optics->name().c_str()));
	m_label->setPos(-0.5*m_label->boundingRect().width()*scale, -rect.height()*0.7);
	m_label->setScale(scale);
}

void OpticsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
/*
	// Draw the bounding rect of the optics
	QPen boundingPen(Qt::blue);
	boundingPen.setCosmetic(true);
	painter->setPen(boundingPen);
	painter->setBrush(QBrush());
	painter->drawRect(boundingRect());
*/
	QColor opticsColor = QColor(153, 209, 247, 150);
	QBrush opticsBrush(opticsColor);
	QPen opticsPen(opticsColor.darker());
	opticsPen.setCosmetic(true);
	painter->setBrush(opticsBrush);
	painter->setPen(opticsPen);
	QPen textPen(Qt::black);

	QPainterPath path;
	QRectF rect = boundingRect();
	QRectF rightRect = rect;
	rightRect.moveLeft(rect.width()/4.);
	QRectF leftRect = rect;
	leftRect.moveRight(-rect.width()/4.);

	if (m_optics->type() == CreateBeamType)
	{
		// no painting !
	}
	else if (m_optics->type() == LensType)
	{
		if (dynamic_cast<const Lens*>(m_optics)->focal() >= 0.)
		{
			path.moveTo(0., rect.top());
			path.arcTo(rect, 90., 180.);
			path.arcTo(rect, 270., 180.);
		}
		else
		{
			path.moveTo(leftRect.center().x(), rect.top());
			path.arcTo(rightRect, 90., 180.);
			path.arcTo(leftRect, 270., 180.);
		}
		painter->drawPath(path);
	}
	else if ((m_optics->type() == FlatInterfaceType) || (m_optics->type() == CurvedInterfaceType))
	{
		double indexRatio = dynamic_cast<const Interface*>(m_optics)->indexRatio();
		double origin = (indexRatio >= 1.) ? rect.right() : rect.left();
		path.moveTo(origin, rect.top());
		path.lineTo(rect.center().x(), rect.top());
		if (m_optics->type() == FlatInterfaceType)
			path.lineTo(rect.center().x(), rect.bottom());
		else
			path.arcTo(rect, 90., dynamic_cast<const CurvedInterface*>(m_optics)->surfaceRadius() >= 0. ? 180. : -180.);
		path.lineTo(origin, rect.bottom());
		painter->drawPath(path);
	}
	else if (m_optics->type() == DielectricSlabType)
	{
		painter->drawRect(rect);
	}
	else if (m_optics->type() == FlatMirrorType)
	{
		painter->setPen(QPen(Qt::black, rect.width()/10., Qt::SolidLine, Qt::RoundCap));
		painter->drawLine(QLineF(0., rect.top(), 0., rect.bottom()));
		const double dx = rect.width()/2.;
		for (double pos = rect.top(); pos < rect.bottom() - dx; pos += dx)
			painter->drawLine(QLineF(0., pos, rect.right(), pos + dx));
	}
	else if (m_optics->type() == CurvedMirrorType)
	{
		painter->setPen(QPen(Qt::black, rect.width()/10., Qt::SolidLine, Qt::RoundCap));
		const double dx = rect.width()/2.;
		if (dynamic_cast<const CurvedMirror*>(m_optics)->curvatureRadius() >= 0.)
			painter->drawArc(rect.translated(-dx, 0.), 4320, 2880);
		else
			painter->drawArc(rect.translated(dx, 0.), 1440, 2880);
		for (double pos = rect.top(); pos < rect.bottom() - dx; pos += dx)
			painter->drawLine(QLineF(0., pos, rect.right(), pos + dx));
	}
	else if (m_optics->type() == GenericABCDType)
	{
		QColor color = QColor(193, 193, 193, 150);
		painter->setBrush(QBrush(color));
		painter->drawRect(rect);
	}
}

/////////////////////////////////////////////////
// BeamView class

BeamItem::BeamItem(const Beam* beam, const Beam* previousBeam, const Beam* nextBeam)
	: QGraphicsItem()
	, m_beam(beam)
	, m_previousBeam(previousBeam)
	, m_nextBeam(nextBeam)
{
	m_drawText = true;
	m_style = true;
	m_auxiliary = false;
	setZValue(0.);
}

void BeamItem::updateTransform()
{
	OpticsScene* opticsScene = dynamic_cast<OpticsScene*>(scene());
	if (!opticsScene)
		return;

	// Find beam start and stop angles
	double startAngle = 0.;
	double stopAngle  = 0.;
	if (m_previousBeam && (fabs(m_previousBeam->angle() - m_beam->angle()) > Utils::epsilon))
		startAngle = (M_PI - m_beam->angle() + m_previousBeam->angle())/2.;
	if (m_nextBeam && (fabs(m_nextBeam->angle() - m_beam->angle()) > Utils::epsilon))
		stopAngle  = (M_PI + m_nextBeam->angle() - m_beam->angle())/2.;

//	if (m_previousBeam && m_nextBeam)
//		qDebug() << m_beam->angle() << m_previousBeam->angle() << m_nextBeam->angle() << startAngle << stopAngle;

	// Update cached information about the beam geometry
	prepareGeometryChange();
	m_orientationCache = opticsScene->orientation();
	std::pair<double, double> bounds;
	if (tan(startAngle) == 0.)
		m_startUpperCache = m_startLowerCache = m_beam->start();
	else
	{
		bounds = m_beam->angledBoundaries(m_beam->start(), -1./(tan(startAngle)*opticsScene->beamScale()), m_orientationCache);
		m_startUpperCache = bounds.first;
		m_startLowerCache = bounds.second;
	}
	if (tan(stopAngle) == 0.)
		m_stopUpperCache = m_stopLowerCache = m_beam->stop();
	else
	{
		bounds = m_beam->angledBoundaries(m_beam->stop(), -1./(tan(stopAngle)*opticsScene->beamScale()), m_orientationCache);
		m_stopUpperCache = bounds.first;
		m_stopLowerCache = bounds.second;
	}

	// Update the beam bounding rect
	/// @todo intersection with scene rect
	const double minStart = qMin(m_startLowerCache, m_startUpperCache);
	const double maxStop  = qMax(m_stopLowerCache, m_stopUpperCache);
	const double maxLowerRadius = qMax(m_beam->radius(m_startLowerCache, m_orientationCache),
									   m_beam->radius(m_stopLowerCache,  m_orientationCache));
	const double maxUpperRadius = qMax(m_beam->radius(m_startUpperCache, m_orientationCache),
									   m_beam->radius(m_stopUpperCache,  m_orientationCache));
	m_boundingRectCache = QRectF(QPointF(minStart, -maxUpperRadius), QPointF(maxStop, maxLowerRadius));

	// Position the beam
	QTransform transform;
	transform.translate(beam()->origin().x(), -beam()->origin().y());
	transform.rotate(-beam()->angle()*180./M_PI);
	transform.scale(1., opticsScene->beamScale());
	setTransform(transform);
}

QRectF BeamItem::boundingRect() const
{
	return m_boundingRectCache;
}

void BeamItem::drawUpperBeamSegment(double start, double stop, double pixel, int nStep, QPolygonF& polygon) const
{
	start = qMax(start, m_startUpperCache);
	stop  = qMin(stop,  m_stopUpperCache);

	if (start >= stop)
		return;

	// "pixel" is added to avoid overlong iterations due to very small steps caused by rounding errors
	double step = qMax((stop - start)/double(nStep) + pixel, pixel);

	// minus sign for the upper beam because the Qt coordinates system points downwoards
	for (double z = start; z < stop; z += step)
		polygon.append(QPointF(z, -m_beam->radius(z, m_orientationCache)));
	polygon.append(QPointF(stop, -m_beam->radius(stop, m_orientationCache)));
}

void BeamItem::drawLowerBeamSegment(double start, double stop, double pixel, int nStep, QPolygonF& polygon) const
{
	start = qMax(start, m_startLowerCache);
	stop  = qMin(stop,  m_stopLowerCache);

	if (start >= stop)
		return;

	// "pixel" is added to avoid overlong iterations due to very small steps caused by rounding errors
	double step = qMax((stop - start)/double(nStep) + pixel, pixel);

	for (double z = stop; z > start; z -= step)
		polygon.append(QPointF(z, m_beam->radius(z, m_orientationCache)));
	polygon.append(QPointF(start, m_beam->radius(start, m_orientationCache)));
}

void BeamItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(widget);
	Q_UNUSED(option);
/*
	QPen boundingPen(Qt::red);
	boundingPen.setCosmetic(true);
	painter->setPen(boundingPen);
	painter->setBrush(QBrush());
	painter->drawRect(boundingRect());
*/
	QColor beamColor = wavelengthColor(m_beam->wavelength());
	beamColor.setAlpha(200);

	QPen beamPen(beamColor);
	if (m_style)
		beamPen = Qt::NoPen;
	else
		beamPen.setCosmetic(true);
	painter->setPen(beamPen);
	painter->setBrush(QBrush(beamColor, m_style ? Qt::SolidPattern : Qt::NoBrush));

	const double horizontalScale = 1./sqrt(sqr(painter->worldTransform().m11()) + sqr(painter->worldTransform().m12())); // m/Pixels
	const double verticalScale   = 1./sqrt(sqr(painter->worldTransform().m22()) + sqr(painter->worldTransform().m21())); // m/Pixels
/*
	qDebug() << "paint" << horizontalScale << verticalScale;
	qDebug() << 1./sqrt(sqr(painter->worldTransform().m11()) + sqr(painter->worldTransform().m12())) <<
				1./sqrt(sqr(painter->worldTransform().m22()) + sqr(painter->worldTransform().m21()));
*/
	const double waist = m_beam->waist(m_orientationCache);
	const double waistPosition = m_beam->waistPosition(m_orientationCache);
	const double rayleigh = m_beam->rayleigh(m_orientationCache);

	// Construct the beam polygon
	QPolygonF beamPolygon;
	// Upper part
	double nextPos, pos = m_startUpperCache;
	if (waist > verticalScale) // The waist is pixel resolved
	{
		drawUpperBeamSegment(pos, nextPos = waistPosition - 5.*rayleigh, horizontalScale, 1,  beamPolygon); pos = nextPos;
		drawUpperBeamSegment(pos, nextPos = waistPosition - rayleigh,    horizontalScale, 10, beamPolygon); pos = nextPos;
		drawUpperBeamSegment(pos, nextPos = waistPosition + rayleigh,    horizontalScale, 20, beamPolygon); pos = nextPos;
		drawUpperBeamSegment(pos, nextPos = waistPosition + 5.*rayleigh, horizontalScale, 10, beamPolygon); pos = nextPos;
		drawUpperBeamSegment(pos, nextPos = m_stopUpperCache,            horizontalScale, 1,  beamPolygon); pos = nextPos;
	}
	else                       // The waist is NOT pixel resolved
	{
		drawUpperBeamSegment(pos, nextPos = waistPosition,    horizontalScale, 1, beamPolygon); pos = nextPos;
		drawUpperBeamSegment(pos, nextPos = m_stopUpperCache, horizontalScale, 1, beamPolygon); pos = nextPos;
	}
	// Lower part
	pos = m_stopLowerCache;
	if (waist > verticalScale) // The waist is pixel resolved
	{
		drawLowerBeamSegment(nextPos = waistPosition + 5.*rayleigh, pos, horizontalScale, 1,  beamPolygon); pos = nextPos;
		drawLowerBeamSegment(nextPos = waistPosition + rayleigh,    pos, horizontalScale, 10, beamPolygon); pos = nextPos;
		drawLowerBeamSegment(nextPos = waistPosition - rayleigh,    pos, horizontalScale, 20, beamPolygon); pos = nextPos;
		drawLowerBeamSegment(nextPos = waistPosition - 5.*rayleigh, pos, horizontalScale, 10, beamPolygon); pos = nextPos;
		drawLowerBeamSegment(nextPos = m_startLowerCache,           pos, horizontalScale, 1,  beamPolygon); pos = nextPos;
	}
	else                       // The waist is NOT pixel resolved
	{
		drawLowerBeamSegment(nextPos = waistPosition,     pos, horizontalScale, 1, beamPolygon); pos = nextPos;
		drawLowerBeamSegment(nextPos = m_startLowerCache, pos, horizontalScale, 1, beamPolygon); pos = nextPos;
	}
	painter->drawConvexPolygon(beamPolygon);
/*
	// Waist label
	QPen textPen(Qt::black);
	if (m_drawText)
	{
		painter->setPen(textPen);
		QPointF waistTop(waistPosition, - waist);
		painter->drawLine(QPointF(waistPosition, 0.), waistTop);
		QString text; text.setNum(round(waist*Units::getUnit(UnitWaist).divider()));
		QRectF textRect(0., 0., 100., 15.);
		textRect.moveCenter(waistTop - QPointF(0., 15.));
		painter->drawText(textRect, Qt::AlignHCenter | Qt::AlignBottom, text);
	}
*/
}
