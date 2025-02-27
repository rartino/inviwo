/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/qtwidgets/tf/tfcontrolpointconnection.h>

#include <modules/qtwidgets/tf/tfeditorcontrolpoint.h>  // for TFEditorControlPoint

#include <QColor>               // for QColor
#include <QGraphicsScene>       // for QGraphicsScene
#include <QPainter>             // for QPainter, QPainter::Antialiasing
#include <QPainterPathStroker>  // for QPainterPathStroker
#include <QPen>                 // for QPen
#include <QPoint>               // for operator==
#include <QRectF>               // for QRectF

class QStyleOptionGraphicsItem;
class QWidget;

namespace inviwo {

TFControlPointConnection::TFControlPointConnection()
    : QGraphicsItem(), left_(nullptr), right_(nullptr), path_(), shape_(), rect_() {
    setZValue(8.0);
    updateShape();
}

TFControlPointConnection::~TFControlPointConnection() = default;

void TFControlPointConnection::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {

    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);

    QPen pathPen(QColor(66, 66, 66));
    pathPen.setWidth(3);
    pathPen.setCosmetic(true);

    p->setPen(pathPen);
    p->drawPath(path_);
    p->restore();
}

void TFControlPointConnection::updateShape() {
    if (left_ == nullptr && right_ == nullptr) {
        path_ = QPainterPath();
    }

    path_ = QPainterPath(getStart());
    path_.lineTo(getStop());

    rect_ = path_.boundingRect();

    QPainterPathStroker pathStrocker;
    pathStrocker.setWidth(10.0);
    shape_ = pathStrocker.createStroke(path_);

    prepareGeometryChange();
    update();
}

QPointF TFControlPointConnection::getStart() const {
    QPointF start;
    if (left_) {
        start = left_->getCurrentPos();
    } else if (right_ && scene()) {
        start = QPointF(scene()->sceneRect().left(), right_->getCurrentPos().y());
    }
    return start;
}

QPointF TFControlPointConnection::getStop() const {
    QPointF stop;
    if (right_) {
        stop = right_->getCurrentPos();
    } else if (left_ && scene()) {
        stop = QPointF(scene()->sceneRect().right(), left_->getCurrentPos().y());
    }
    return stop;
}

QRectF TFControlPointConnection::boundingRect() const { return rect_; }

QPainterPath TFControlPointConnection::shape() const { return shape_; }

bool operator==(const TFControlPointConnection& lhs, const TFControlPointConnection& rhs) {
    return lhs.getStart() == rhs.getStart() && lhs.getStop() == rhs.getStop();
}

bool operator!=(const TFControlPointConnection& lhs, const TFControlPointConnection& rhs) {
    return !operator==(lhs, rhs);
}

bool operator<(const TFControlPointConnection& lhs, const TFControlPointConnection& rhs) {
    return 0.5f * (lhs.getStart().x() + lhs.getStop().x()) <
           0.5f * (rhs.getStart().x() + rhs.getStop().x());
}

bool operator>(const TFControlPointConnection& lhs, const TFControlPointConnection& rhs) {
    return rhs < lhs;
}

bool operator<=(const TFControlPointConnection& lhs, const TFControlPointConnection& rhs) {
    return !(rhs < lhs);
}

bool operator>=(const TFControlPointConnection& lhs, const TFControlPointConnection& rhs) {
    return !(lhs < rhs);
}

}  // namespace inviwo
