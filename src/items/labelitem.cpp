#include "labelitem.h"

#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QStyleOptionGraphicsItem>
#include <QtMath>
#include <QUuid>

// ============ LabelItem 实现 ============

LabelItem::LabelItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_rotation(0.0)
    , m_locked(false)
    , m_visible(true)
    , m_modified(false)
    , m_hovered(false)
    , m_dragging(false)
    , m_activeHandle(-1)
{
    // 设置标志
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);

    // 设置接受悬停事件
    setAcceptHoverEvents(true);

    // 设置缓存模式以提高性能
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    // 初始化矩形区域
    m_rect = QRectF(0, 0, 100, 50);

    // 生成唯一ID
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // 设置默认名称
    m_name = "Item";

    // 设置默认光标
    setCursor(Qt::ArrowCursor);
}

LabelItem::~LabelItem()
{
    // 清理资源（如有必要）
}

QRectF LabelItem::boundingRect() const
{
    // 添加余量以容纳控制点和选中框
    qreal margin = HandleSize + HandleSpace;
    QRectF rect = m_rect.adjusted(-margin, -margin, margin, margin);

    // 如果有旋转角度，需要更大的范围
    if (m_rotation != 0.0) {
        // 找出旋转后矩形的最大尺寸
        qreal diagonal = qSqrt(rect.width() * rect.width() + rect.height() * rect.height());
        return QRectF(-diagonal/2, -diagonal/2, diagonal, diagonal);
    }

    return rect;
}

void LabelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    // 保存当前状态
    painter->save();

    // 应用旋转变换
    painter->translate(m_rect.center());
    painter->rotate(m_rotation);
    painter->translate(-m_rect.center());

    // 如果元素被选中，绘制选中效果
    if (option->state & QStyle::State_Selected) {
        drawSelection(painter);

        // 如果没有锁定，绘制控制点
        if (!m_locked) {
            drawHandles(painter);
        }
    }

    // 如果鼠标悬停在元素上，绘制悬停效果
    if (m_hovered && !m_locked) {
        painter->setPen(QPen(Qt::gray, 1, Qt::DashLine));
        painter->drawRect(m_rect);
    }

    // 恢复状态
    painter->restore();
}

bool LabelItem::contains(const QPointF &point) const
{
    // 如果有旋转，需要将点转换回元素坐标系
    if (m_rotation != 0.0) {
        QPointF center = m_rect.center();
        QPointF rotatedPoint = point - center;

        // 应用反向旋转变换
        qreal angle = -m_rotation * M_PI / 180.0;
        qreal x = rotatedPoint.x() * qCos(angle) - rotatedPoint.y() * qSin(angle);
        qreal y = rotatedPoint.x() * qSin(angle) + rotatedPoint.y() * qCos(angle);

        rotatedPoint = QPointF(x, y) + center;
        return m_rect.contains(rotatedPoint);
    }

    return m_rect.contains(point);
}

void LabelItem::setPosition(const QPointF &pos)
{
    prepareGeometryChange();
    m_rect.moveTopLeft(pos);
    setModified(true);
    updateHandles();
    update();
    emit positionChanged(pos);
    emit itemChanged();
}

QPointF LabelItem::position() const
{
    return m_rect.topLeft();
}

void LabelItem::setSize(const QSizeF &size)
{
    if (size.width() <= 0 || size.height() <= 0) {
        return;  // 防止无效尺寸
    }

    prepareGeometryChange();
    m_rect.setSize(size);
    setModified(true);
    updateHandles();
    update();
    emit sizeChanged(size);
    emit itemChanged();
}

QSizeF LabelItem::size() const
{
    return m_rect.size();
}

void LabelItem::setRotation(qreal angle)
{
    if (m_rotation == angle) {
        return;
    }

    // 规范化角度到0-360
    while (angle >= 360.0) {
        angle -= 360.0;
    }
    while (angle < 0.0) {
        angle += 360.0;
    }

    prepareGeometryChange();
    m_rotation = angle;
    setModified(true);
    update();
    emit rotationChanged(angle);
    emit itemChanged();
}

qreal LabelItem::rotation() const
{
    return m_rotation;
}

void LabelItem::setLocked(bool locked)
{
    if (m_locked == locked) {
        return;
    }

    m_locked = locked;
    setFlag(QGraphicsItem::ItemIsMovable, !locked);
    setModified(true);
    update();
    emit lockedChanged(locked);
    emit itemChanged();
}

bool LabelItem::isLocked() const
{
    return m_locked;
}

void LabelItem::setVisible(bool visible)
{
    if (m_visible == visible) {
        return;
    }

    m_visible = visible;
    QGraphicsItem::setVisible(visible);
    setModified(true);
    emit itemChanged();
}

bool LabelItem::isVisible() const
{
    return m_visible;
}

void LabelItem::setId(const QString &id)
{
    if (m_id == id) {
        return;
    }

    m_id = id;
    emit idChanged(id);
}

QString LabelItem::id() const
{
    return m_id;
}

void LabelItem::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }

    m_name = name;
    setModified(true);
    emit nameChanged(name);
    emit itemChanged();
}

QString LabelItem::name() const
{
    return m_name;
}

void LabelItem::setModified(bool modified)
{
    m_modified = modified;
}

bool LabelItem::isModified() const
{
    return m_modified;
}

void LabelItem::moveBy(qreal dx, qreal dy)
{
    QPointF newPos = position() + QPointF(dx, dy);
    setPosition(newPos);
}

void LabelItem::resize(qreal width, qreal height)
{
    setSize(QSizeF(width, height));
}

void LabelItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_locked) {
        event->ignore();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragStartPos = event->pos();
        m_lastPos = event->pos();
        m_startRect = m_rect;

        // 检查是否点击了控制点
        m_activeHandle = handleAtPosition(event->pos());

        if (m_activeHandle != -1) {
            // 如果点击了控制点，设置适当的光标
            switch (m_activeHandle) {
                case HandleTopLeft:
                case HandleBottomRight:
                    setCursor(Qt::SizeFDiagCursor);
                    break;
                case HandleTopRight:
                case HandleBottomLeft:
                    setCursor(Qt::SizeBDiagCursor);
                    break;
                case HandleTopMiddle:
                case HandleBottomMiddle:
                    setCursor(Qt::SizeVerCursor);
                    break;
                case HandleMiddleLeft:
                case HandleMiddleRight:
                    setCursor(Qt::SizeHorCursor);
                    break;
                case HandleRotate:
                    setCursor(Qt::CrossCursor);
                    break;
            }
        } else {
            setCursor(Qt::ClosedHandCursor);
        }

        // 确保项目被选择
        setSelected(true);

        event->accept();
    }

    QGraphicsItem::mousePressEvent(event);
}

void LabelItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_dragging || m_locked) {
        QGraphicsItem::mouseMoveEvent(event);
        return;
    }

    QPointF currentPos = event->pos();
    QPointF delta = currentPos - m_lastPos;

    if (m_activeHandle == -1) {
        // 移动整个项目
        QGraphicsItem::mouseMoveEvent(event);
    } else {
        // 调整大小或旋转
        prepareGeometryChange();

        if (m_activeHandle == HandleRotate) {
            // 旋转操作
            QPointF center = m_rect.center();
            QPointF startVector = m_dragStartPos - center;
            QPointF currentVector = currentPos - center;

            // 计算角度变化
            qreal startAngle = qAtan2(startVector.y(), startVector.x()) * 180.0 / M_PI;
            qreal currentAngle = qAtan2(currentVector.y(), currentVector.x()) * 180.0 / M_PI;
            qreal deltaAngle = currentAngle - startAngle;

            // 应用新的旋转角度
            setRotation(m_rotation + deltaAngle);

            // 更新拖动起始点
            m_dragStartPos = currentPos;
        } else {
            // 调整大小操作
            QRectF newRect = m_rect;

            switch (m_activeHandle) {
                case HandleTopLeft:
                    newRect.setTopLeft(newRect.topLeft() + delta);
                    break;
                case HandleTopMiddle:
                    newRect.setTop(newRect.top() + delta.y());
                    break;
                case HandleTopRight:
                    newRect.setTopRight(newRect.topRight() + delta);
                    break;
                case HandleMiddleLeft:
                    newRect.setLeft(newRect.left() + delta.x());
                    break;
                case HandleMiddleRight:
                    newRect.setRight(newRect.right() + delta.x());
                    break;
                case HandleBottomLeft:
                    newRect.setBottomLeft(newRect.bottomLeft() + delta);
                    break;
                case HandleBottomMiddle:
                    newRect.setBottom(newRect.bottom() + delta.y());
                    break;
                case HandleBottomRight:
                    newRect.setBottomRight(newRect.bottomRight() + delta);
                    break;
            }

            // 确保矩形有效
            if (newRect.width() >= 10 && newRect.height() >= 10) {
                m_rect = newRect;
                updateHandles();
                update();
            }
        }
    }

    m_lastPos = currentPos;

    // 标记为已修改
    setModified(true);
    emit itemChanged();

    event->accept();
}

void LabelItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        m_activeHandle = -1;
        setCursor(Qt::ArrowCursor);

        // 发出选中信号
        emit itemSelected();

        event->accept();
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

void LabelItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_locked) {
        m_hovered = true;
        update();
    }

    QGraphicsItem::hoverEnterEvent(event);
}

void LabelItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_hovered = false;
    setCursor(Qt::ArrowCursor);
    update();

    QGraphicsItem::hoverLeaveEvent(event);
}

void LabelItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_locked) {
        setCursor(Qt::ForbiddenCursor);
        QGraphicsItem::hoverMoveEvent(event);
        return;
    }

    if (isSelected()) {
        // 检查是否悬停在控制点上
        int handle = handleAtPosition(event->pos());

        // 根据控制点设置合适的光标
        if (handle != -1) {
            switch (handle) {
                case HandleTopLeft:
                case HandleBottomRight:
                    setCursor(Qt::SizeFDiagCursor);
                    break;
                case HandleTopRight:
                case HandleBottomLeft:
                    setCursor(Qt::SizeBDiagCursor);
                    break;
                case HandleTopMiddle:
                case HandleBottomMiddle:
                    setCursor(Qt::SizeVerCursor);
                    break;
                case HandleMiddleLeft:
                case HandleMiddleRight:
                    setCursor(Qt::SizeHorCursor);
                    break;
                case HandleRotate:
                    setCursor(Qt::CrossCursor);
                    break;
            }
        } else {
            setCursor(Qt::ArrowCursor);
        }
    } else {
        setCursor(Qt::ArrowCursor);
    }

    QGraphicsItem::hoverMoveEvent(event);
}

void LabelItem::updateHandles()
{
    // 可以在子类中重写以定制控制点
}

int LabelItem::handleAtPosition(const QPointF &point) const
{
    if (!isSelected()) {
        return -1;
    }

    QPointF center = m_rect.center();

    // 如果有旋转，需要将点转换回元素坐标系
    QPointF transformedPoint = point;
    if (m_rotation != 0.0) {
        QPointF relativePoint = point - center;

        // 应用反向旋转变换
        qreal angle = -m_rotation * M_PI / 180.0;
        qreal x = relativePoint.x() * qCos(angle) - relativePoint.y() * qSin(angle);
        qreal y = relativePoint.x() * qSin(angle) + relativePoint.y() * qCos(angle);

        transformedPoint = QPointF(x, y) + center;
    }

    // 检查旋转控制点
    QPointF rotateHandle(center.x(), m_rect.top() - RotateHandleDistance);
    if (QRectF(rotateHandle.x() - HandleSize/2, rotateHandle.y() - HandleSize/2,
               HandleSize, HandleSize).contains(transformedPoint)) {
        return HandleRotate;
    }

    // 检查其他控制点
    QVector<QPointF> handles;
    handles << m_rect.topLeft()
            << QPointF(center.x(), m_rect.top())
            << m_rect.topRight()
            << QPointF(m_rect.left(), center.y())
            << QPointF(m_rect.right(), center.y())
            << m_rect.bottomLeft()
            << QPointF(center.x(), m_rect.bottom())
            << m_rect.bottomRight();

    for (int i = 0; i < handles.size(); ++i) {
        if (QRectF(handles[i].x() - HandleSize/2, handles[i].y() - HandleSize/2,
                   HandleSize, HandleSize).contains(transformedPoint)) {
            return i;
        }
    }

    return -1;
}

void LabelItem::drawSelection(QPainter *painter) const
{
    // 绘制选中框
    painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(m_rect);
}

void LabelItem::drawHandles(QPainter *painter) const
{
    painter->setPen(QPen(Qt::blue, 1, Qt::SolidLine));
    painter->setBrush(QBrush(Qt::white));

    QPointF center = m_rect.center();

    // 绘制8个调整大小的控制点
    QVector<QPointF> handles;
    handles << m_rect.topLeft()
            << QPointF(center.x(), m_rect.top())
            << m_rect.topRight()
            << QPointF(m_rect.left(), center.y())
            << QPointF(m_rect.right(), center.y())
            << m_rect.bottomLeft()
            << QPointF(center.x(), m_rect.bottom())
            << m_rect.bottomRight();

    for (const QPointF &handle : handles) {
        painter->drawRect(QRectF(handle.x() - HandleSize/2, handle.y() - HandleSize/2,
                                 HandleSize, HandleSize));
    }

    // 绘制旋转控制点
    QPointF rotateHandle(center.x(), m_rect.top() - RotateHandleDistance);
    painter->drawLine(QPointF(center.x(), m_rect.top()), rotateHandle);
    painter->setBrush(QBrush(Qt::green));
    painter->drawEllipse(rotateHandle, HandleSize/2, HandleSize/2);
}

// ============ MoveItemCommand 实现 ============

MoveItemCommand::MoveItemCommand(LabelItem *item, const QPointF &oldPos, const QPointF &newPos)
    : QUndoCommand(QObject::tr("移动 %1").arg(item->name()))
    , m_item(item)
    , m_oldPos(oldPos)
    , m_newPos(newPos)
{
}

void MoveItemCommand::redo()
{
    m_item->setPosition(m_newPos);
}

void MoveItemCommand::undo()
{
    m_item->setPosition(m_oldPos);
}

// ============ ResizeItemCommand 实现 ============

ResizeItemCommand::ResizeItemCommand(LabelItem *item, const QRectF &oldRect, const QRectF &newRect)
    : QUndoCommand(QObject::tr("调整 %1 大小").arg(item->name()))
    , m_item(item)
    , m_oldRect(oldRect)
    , m_newRect(newRect)
{
}

void ResizeItemCommand::redo()
{
    m_item->setPosition(m_newRect.topLeft());
    m_item->setSize(m_newRect.size());
}

void ResizeItemCommand::undo()
{
    m_item->setPosition(m_oldRect.topLeft());
    m_item->setSize(m_oldRect.size());
}

// ============ RotateItemCommand 实现 ============

RotateItemCommand::RotateItemCommand(LabelItem *item, qreal oldAngle, qreal newAngle)
    : QUndoCommand(QObject::tr("旋转 %1").arg(item->name()))
    , m_item(item)
    , m_oldAngle(oldAngle)
    , m_newAngle(newAngle)
{
}

void RotateItemCommand::redo()
{
    m_item->setRotation(m_newAngle);
}

void RotateItemCommand::undo()
{
    m_item->setRotation(m_oldAngle);
}