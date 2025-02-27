#include "labeleditview.h"
#include "../models/labelmodels.h"
#include "../items/labelitem.h"
#include "../items/textitem.h"
#include "../items/imageitem.h"
#include "../items/barcodeitem.h"
#include "../items/qrcodeitem.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QScrollBar>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QContextMenuEvent>
#include <QMimeData>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

// 常量定义
const qreal MIN_ZOOM = 0.1;
const qreal MAX_ZOOM = 5.0;
const qreal ZOOM_STEP = 0.1;
const int GRID_SIZE = 10; // 网格大小（像素）
const int RULER_SIZE = 20; // 标尺大小（像素）

LabelEditView::LabelEditView(QWidget *parent)
    : QGraphicsView(parent)
    , m_scene(nullptr)
    , m_document(nullptr)
    , m_zoom(1.0)
    , m_gridVisible(true)
    , m_selecting(false)
    , m_selectionRect(nullptr)
    , m_gridSize(GRID_SIZE)
    , m_snapToGrid(true)
    , m_rulersVisible(true)
    , m_selectedItemsGroup(nullptr)
    , m_movingItems(false)
{
    // 初始化场景
    initScene();

    // 设置视图属性
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::TextAntialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setDragMode(QGraphicsView::RubberBandDrag);
    setAcceptDrops(true);

    // 设置背景色
    setBackgroundBrush(QBrush(Qt::gray));

    // 设置聚焦策略
    setFocusPolicy(Qt::StrongFocus);
}

LabelEditView::~LabelEditView()
{
    // 清理选择矩形
    if (m_selectionRect) {
        m_scene->removeItem(m_selectionRect);
        delete m_selectionRect;
    }

    // 清理选中元素组
    if (m_selectedItemsGroup) {
        m_scene->destroyItemGroup(m_selectedItemsGroup);
    }
}

void LabelEditView::setDocument(LabelDocument *document)
{
    if (m_document == document) {
        return;
    }

    // 断开旧文档的信号连接
    if (m_document) {
        disconnect(m_document, nullptr, this, nullptr);
    }

    m_document = document;

    // 如果有新文档，连接信号并设置场景
    if (m_document) {
        // 连接文档信号
        connect(m_document, &LabelDocument::itemAdded, this, &LabelEditView::updateView);
        connect(m_document, &LabelDocument::itemRemoved, this, &LabelEditView::updateView);
        connect(m_document, &LabelDocument::itemChanged, this, &LabelEditView::updateView);
        connect(m_document, &LabelDocument::pageSizeChanged, this, &LabelEditView::updateView);
        connect(m_document, &LabelDocument::orientationChanged, this, &LabelEditView::updateView);
        connect(m_document, &LabelDocument::marginsChanged, this, &LabelEditView::updateView);

        // 设置场景
        m_document->setScene(m_scene);

        // 更新标签显示
        updateLabelDisplay();
    }
}

LabelDocument* LabelEditView::document() const
{
    return m_document;
}

QList<LabelItem*> LabelEditView::selectedItems() const
{
    QList<LabelItem*> items;

    if (!m_scene) {
        return items;
    }

    QList<QGraphicsItem*> selectedGraphicsItems = m_scene->selectedItems();
    for (QGraphicsItem *item : selectedGraphicsItems) {
        LabelItem *labelItem = dynamic_cast<LabelItem*>(item);
        if (labelItem) {
            items.append(labelItem);
        }
    }

    return items;
}

qreal LabelEditView::currentZoom() const
{
    return m_zoom;
}

void LabelEditView::zoomIn()
{
    qreal newZoom = m_zoom + ZOOM_STEP;
    setZoom(qMin(newZoom, MAX_ZOOM));
}

void LabelEditView::zoomOut()
{
    qreal newZoom = m_zoom - ZOOM_STEP;
    setZoom(qMax(newZoom, MIN_ZOOM));
}

void LabelEditView::zoomReset()
{
    setZoom(1.0);
}

void LabelEditView::setZoom(qreal zoom)
{
    if (qFuzzyCompare(m_zoom, zoom)) {
        return;
    }

    m_zoom = zoom;

    // 重置变换
    resetTransform();

    // 应用缩放
    scale(m_zoom, m_zoom);

    // 更新视口
    viewport()->update();

    // 发出缩放改变信号
    emit zoomChanged(m_zoom);
}

void LabelEditView::zoomToFit()
{
    if (!m_document) {
        return;
    }

    QSizeF labelSize = m_document->pageRealSize();
    QSizeF viewSize = viewport()->size();

    // 计算适合视口的缩放比例
    qreal zoomX = viewSize.width() / labelSize.width();
    qreal zoomY = viewSize.height() / labelSize.height();
    qreal zoom = qMin(zoomX, zoomY) * 0.9; // 留出一些边距

    // 限制缩放范围
    zoom = qBound(MIN_ZOOM, zoom, MAX_ZOOM);

    // 设置缩放
    setZoom(zoom);

    // 确保标签居中
    centerOn(labelSize.width() / 2, labelSize.height() / 2);
}

void LabelEditView::selectAll()
{
    if (!m_document) {
        return;
    }

    // 选中所有元素
    for (LabelItem *item : m_document->items()) {
        item->setSelected(true);
    }

    // 更新选择状态
    updateSelectionState();
}

void LabelEditView::deselectAll()
{
    if (!m_scene) {
        return;
    }

    // 取消选择所有元素
    m_scene->clearSelection();

    // 更新选择状态
    updateSelectionState();
}

void LabelEditView::deleteSelectedItems()
{
    QList<LabelItem*> items = selectedItems();
    if (items.isEmpty() || !m_document) {
        return;
    }

    // 确认删除
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("删除元素"),
        tr("确定要删除选中的 %1 个元素吗?").arg(items.size()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    // 使用撤销栈删除元素
    if (m_document->undoStack()) {
        QUndoCommand *deleteCommand = new QUndoCommand(tr("删除 %1 个元素").arg(items.size()));

        for (LabelItem *item : items) {
            new RemoveItemCommand(m_document, item);
        }

        m_document->undoStack()->push(deleteCommand);
    } else {
        // 直接删除元素
        for (LabelItem *item : items) {
            m_document->removeItem(item);
        }
    }

    // 更新选择状态
    updateSelectionState();
}

void LabelEditView::cutSelectedItems()
{
    QList<LabelItem*> items = selectedItems();
    if (items.isEmpty() || !m_document) {
        return;
    }

    // 先复制
    copySelectedItems();

    // 然后删除
    if (m_document->undoStack()) {
        QUndoCommand *cutCommand = new QUndoCommand(tr("剪切 %1 个元素").arg(items.size()));

        for (LabelItem *item : items) {
            new RemoveItemCommand(m_document, item);
        }

        m_document->undoStack()->push(cutCommand);
    } else {
        // 直接删除元素
        for (LabelItem *item : items) {
            m_document->removeItem(item);
        }
    }

    // 更新选择状态
    updateSelectionState();
}

void LabelEditView::copySelectedItems()
{
    QList<LabelItem*> items = selectedItems();
    if (items.isEmpty()) {
        return;
    }

    // 创建MIME数据
    QMimeData *mimeData = createMimeDataFromItems(items);

    // 设置到剪贴板
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData);
}

void LabelEditView::pasteItems()
{
    if (!m_document) {
        return;
    }

    // 获取剪贴板数据
    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    // 获取视图中心点
    QPointF centerPos = mapToScene(viewport()->rect().center());

    // 处理粘贴的数据
    handleDroppedData(mimeData, centerPos);
}

LabelItem* LabelEditView::addTextElement(const QPointF &pos)
{
    if (!m_document) {
        return nullptr;
    }

    // 确定位置
    QPointF itemPos = pos.isNull() ? mapToScene(viewport()->rect().center()) : pos;

    // 如果启用了网格对齐，对齐到网格
    if (m_snapToGrid) {
        itemPos.setX(qRound(itemPos.x() / m_gridSize) * m_gridSize);
        itemPos.setY(qRound(itemPos.y() / m_gridSize) * m_gridSize);
    }

    // 创建文本元素
    return m_document->createTextItem(QString(), itemPos);
}

LabelItem* LabelEditView::addImageElement(const QString &imagePath, const QPointF &pos)
{
    if (!m_document) {
        return nullptr;
    }

    // 确定图像路径
    QString path = imagePath;
    if (path.isEmpty()) {
        path = QFileDialog::getOpenFileName(this, tr("选择图像"),
            QDir::homePath(), tr("图像文件 (*.png *.jpg *.jpeg *.bmp *.gif)"));

        if (path.isEmpty()) {
            return nullptr;
        }
    }

    // 确定位置
    QPointF itemPos = pos.isNull() ? mapToScene(viewport()->rect().center()) : pos;

    // 如果启用了网格对齐，对齐到网格
    if (m_snapToGrid) {
        itemPos.setX(qRound(itemPos.x() / m_gridSize) * m_gridSize);
        itemPos.setY(qRound(itemPos.y() / m_gridSize) * m_gridSize);
    }

    // 创建图像元素
    return m_document->createImageItem(path, itemPos);
}

LabelItem* LabelEditView::addBarcodeElement(const QPointF &pos)
{
    if (!m_document) {
        return nullptr;
    }

    // 确定位置
    QPointF itemPos = pos.isNull() ? mapToScene(viewport()->rect().center()) : pos;

    // 如果启用了网格对齐，对齐到网格
    if (m_snapToGrid) {
        itemPos.setX(qRound(itemPos.x() / m_gridSize) * m_gridSize);
        itemPos.setY(qRound(itemPos.y() / m_gridSize) * m_gridSize);
    }

    // 请求条形码数据
    bool ok;
    QString data = QInputDialog::getText(this, tr("条形码数据"),
                                        tr("请输入条形码数据:"), QLineEdit::Normal,
                                        "12345678", &ok);

    if (!ok || data.isEmpty()) {
        return nullptr;
    }

    // 创建条形码元素
    return m_document->createBarcodeItem(data, itemPos);
}

LabelItem* LabelEditView::addQRCodeElement(const QPointF &pos)
{
    if (!m_document) {
        return nullptr;
    }

    // 确定位置
    QPointF itemPos = pos.isNull() ? mapToScene(viewport()->rect().center()) : pos;

    // 如果启用了网格对齐，对齐到网格
    if (m_snapToGrid) {
        itemPos.setX(qRound(itemPos.x() / m_gridSize) * m_gridSize);
        itemPos.setY(qRound(itemPos.y() / m_gridSize) * m_gridSize);
    }

    // 请求二维码数据
    bool ok;
    QString data = QInputDialog::getText(this, tr("二维码数据"),
                                        tr("请输入二维码数据:"), QLineEdit::Normal,
                                        "https://example.com", &ok);

    if (!ok || data.isEmpty()) {
        return nullptr;
    }

    // 创建二维码元素
    return m_document->createQRCodeItem(data, itemPos);
}

void LabelEditView::updateView()
{
    updateLabelDisplay();
    viewport()->update();
}

void LabelEditView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    updateLabelDisplay();
}

void LabelEditView::mousePressEvent(QMouseEvent *event)
{
    if (!m_scene || !m_document) {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    // 保存鼠标位置
    m_lastMousePos = event->pos();
    QPointF scenePos = mapToScene(event->pos());

    // 更新标尺标记
    m_rulerMarker = scenePos;
    emit rulerMarkerChanged(m_rulerMarker);

    // 如果点击了空白区域，开始选择矩形
    if (event->button() == Qt::LeftButton && !itemAt(event->pos())) {
        m_selecting = true;
        m_selectionStart = scenePos;

        // 创建选择矩形
        if (!m_selectionRect) {
            m_selectionRect = new QGraphicsRectItem();
            m_selectionRect->setPen(QPen(Qt::blue, 1, Qt::DashLine));
            m_selectionRect->setBrush(QBrush(QColor(0, 0, 255, 50)));
            m_scene->addItem(m_selectionRect);
        }

        m_selectionRect->setRect(QRectF(m_selectionStart, QSizeF(0, 0)));
        m_selectionRect->show();

        event->accept();
        return;
    }

    // 如果点击了元素，准备移动
    if (event->button() == Qt::LeftButton) {
        QList<QGraphicsItem*> itemsAtPos = m_scene->items(scenePos);
        if (!itemsAtPos.isEmpty()) {
            for (QGraphicsItem *item : itemsAtPos) {
                if (dynamic_cast<LabelItem*>(item)) {
                    m_movingItems = true;
                    m_moveStart = scenePos;
                    break;
                }
            }
        }
    }

    QGraphicsView::mousePressEvent(event);
}

void LabelEditView::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_scene) {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    QPointF scenePos = mapToScene(event->pos());

    // 更新鼠标位置信号
    emit mousePositionChanged(scenePos);

    // 更新标尺标记
    m_rulerMarker = scenePos;
    emit rulerMarkerChanged(m_rulerMarker);

    // 如果正在选择，更新选择矩形
    if (m_selecting && m_selectionRect) {
        QRectF rect = QRectF(m_selectionStart, scenePos).normalized();
        m_selectionRect->setRect(rect);

        // 选择矩形内的元素
        QPainterPath path;
        path.addRect(rect);
        m_scene->setSelectionArea(path, Qt::IntersectsItemShape);

        // 更新选择状态
        updateSelectionState();

        event->accept();
        return;
    }

    // 如果正在移动元素且启用了网格对齐
    if (m_movingItems && m_snapToGrid) {
        // 计算应该移动的距离
        QPointF delta = scenePos - m_lastMousePos;

        // 对齐到网格
        QList<LabelItem*> items = selectedItems();
        for (LabelItem *item : items) {
            QPointF pos = item->position() + delta;
            pos.setX(qRound(pos.x() / m_gridSize) * m_gridSize);
            pos.setY(qRound(pos.y() / m_gridSize) * m_gridSize);
            item->setPosition(pos);
        }

        m_lastMousePos = scenePos;
        event->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void LabelEditView::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_scene) {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }

    // 如果正在选择，结束选择
    if (m_selecting && m_selectionRect) {
        // 隐藏选择矩形
        m_selectionRect->hide();
        m_selecting = false;

        // 更新选择状态
        updateSelectionState();

        event->accept();
    }

    // 如果正在移动元素，结束移动
    if (m_movingItems) {
        m_movingItems = false;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void LabelEditView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QGraphicsView::mouseDoubleClickEvent(event);
}

void LabelEditView::wheelEvent(QWheelEvent *event)
{
    // 如果按住Ctrl键，缩放视图
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->angleDelta().y() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
        event->accept();
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void LabelEditView::keyPressEvent(QKeyEvent *event)
{
    // 处理键盘快捷键
    switch (event->key()) {
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            deleteSelectedItems();
            event->accept();
            return;

        case Qt::Key_A:
            if (event->modifiers() & Qt::ControlModifier) {
                selectAll();
                event->accept();
                return;
            }
            break;

        case Qt::Key_Escape:
            deselectAll();
            event->accept();
            return;

        case Qt::Key_Plus:
        case Qt::Key_Equal:
            if (event->modifiers() & Qt::ControlModifier) {
                zoomIn();
                event->accept();
                return;
            }
            break;

        case Qt::Key_Minus:
            if (event->modifiers() & Qt::ControlModifier) {
                zoomOut();
                event->accept();
                return;
            }
            break;

        case Qt::Key_0:
            if (event->modifiers() & Qt::ControlModifier) {
                zoomReset();
                event->accept();
                return;
            }
            break;

        case Qt::Key_C:
            if (event->modifiers() & Qt::ControlModifier) {
                copySelectedItems();
                event->accept();
                return;
            }
            break;

        case Qt::Key_X:
            if (event->modifiers() & Qt::ControlModifier) {
                cutSelectedItems();
                event->accept();
                return;
            }
            break;

        case Qt::Key_V:
            if (event->modifiers() & Qt::ControlModifier) {
                pasteItems();
                event->accept();
                return;
            }
            break;

        case Qt::Key_G:
            if (event->modifiers() & Qt::ControlModifier) {
                m_gridVisible = !m_gridVisible;
                viewport()->update();
                event->accept();
                return;
            }
            break;

        case Qt::Key_R:
            if (event->modifiers() & Qt::ControlModifier) {
                m_rulersVisible = !m_rulersVisible;
                viewport()->update();
                event->accept();
                return;
            }
            break;

        case Qt::Key_S:
            if (event->modifiers() & Qt::ControlModifier) {
                m_snapToGrid = !m_snapToGrid;
                emit viewStatusChanged(m_snapToGrid ? tr("已启用网格对齐") : tr("已禁用网格对齐"));
                event->accept();
                return;
            }
            break;

        // 方向键移动选中的元素
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
            {
                QList<LabelItem*> items = selectedItems();
                if (!items.isEmpty()) {
                    int dx = 0, dy = 0;
                    if (event->key() == Qt::Key_Left) dx = -1;
                    else if (event->key() == Qt::Key_Right) dx = 1;
                    else if (event->key() == Qt::Key_Up) dy = -1;
                    else if (event->key() == Qt::Key_Down) dy = 1;

                    // 如果按住Shift键，移动10个单位
                    if (event->modifiers() & Qt::ShiftModifier) {
                        dx *= 10;
                        dy *= 10;
                    }

                    // 移动选中的元素
                    for (LabelItem *item : items) {
                        item->moveBy(dx, dy);
                    }

                    event->accept();
                    return;
                }
            }
            break;
    }

    QGraphicsView::keyPressEvent(event);
}

void LabelEditView::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);

    if (!m_document) {
        return;
    }

    // 获取标签尺寸
    QSizeF labelSize = m_document->pageRealSize();

    // 绘制标签背景
    QRectF labelRect(0, 0, labelSize.width(), labelSize.height());

    painter->fillRect(labelRect, Qt::white);
    painter->setPen(QPen(Qt::black, 1));
    painter->drawRect(labelRect);

    // 如果启用网格，绘制网格
    if (m_gridVisible) {
        drawGrid(painter, labelRect);
    }

    // 如果启用标尺，绘制标尺
    if (m_rulersVisible) {
        drawRulers(painter, rect);
    }
}

void LabelEditView::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawForeground(painter, rect);

    // 绘制标尺标记
    if (m_rulersVisible && m_document) {
        QSizeF labelSize = m_document->pageRealSize();
        QRectF labelRect(0, 0, labelSize.width(), labelSize.height());

        if (labelRect.contains(m_rulerMarker)) {
            painter->setPen(QPen(Qt::red, 1, Qt::DashLine));

            // 绘制水平标记线
            painter->drawLine(QPointF(0, m_rulerMarker.y()),
                            QPointF(labelSize.width(), m_rulerMarker.y()));

            // 绘制垂直标记线
            painter->drawLine(QPointF(m_rulerMarker.x(), 0),
                            QPointF(m_rulerMarker.x(), labelSize.height()));
        }
    }
}

void LabelEditView::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_scene || !m_document) {
        QGraphicsView::contextMenuEvent(event);
        return;
    }

    // 获取场景坐标
    QPointF scenePos = mapToScene(event->pos());

    // 获取点击位置的元素
    QList<QGraphicsItem*> itemsAtPos = m_scene->items(scenePos);
    QList<LabelItem*> labelItems;

    for (QGraphicsItem *item : itemsAtPos) {
        LabelItem *labelItem = dynamic_cast<LabelItem*>(item);
        if (labelItem) {
            labelItems.append(labelItem);
        }
    }

    // 如果没有选中的元素，使用点击位置的元素
    if (labelItems.isEmpty()) {
        showContextMenu(event->globalPos(), QList<LabelItem*>());
    } else {
        showContextMenu(event->globalPos(), labelItems);
    }

    event->accept();
}

void LabelEditView::dragEnterEvent(QDragEnterEvent *event)
{
    // 检查是否支持的MIME类型
    if (event->mimeData()->hasUrls() ||
        event->mimeData()->hasImage() ||
        event->mimeData()->hasText() ||
        event->mimeData()->hasFormat("application/x-labelitem")) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void LabelEditView::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void LabelEditView::dropEvent(QDropEvent *event)
{
    if (!m_document) {
        event->ignore();
        return;
    }

    // 获取放下位置
    QPointF pos = mapToScene(event->pos());

    // 处理拖放数据
    handleDroppedData(event->mimeData(), pos);

    event->acceptProposedAction();
}

void LabelEditView::initScene()
{
    // 创建场景
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    // 连接场景的选择改变信号
    connect(m_scene, &QGraphicsScene::selectionChanged, this, [this]() {
        emit selectionChanged();
    });
}

void LabelEditView::updateLabelDisplay()
{
    if (!m_document || !m_scene) {
        return;
    }

    // 获取标签尺寸
    QSizeF labelSize = m_document->pageRealSize();

    // 设置场景大小
    m_scene->setSceneRect(0, 0, labelSize.width(), labelSize.height());

    // 更新视图
    viewport()->update();
}

QPointF LabelEditView::viewToScenePos(const QPointF &pos) const
{
    return mapToScene(pos.toPoint());
}

void LabelEditView::drawGrid(QPainter *painter, const QRectF &rect) const
{
    painter->save();

    // 设置网格笔
    painter->setPen(QPen(QColor(200, 200, 200), 0.5));

    // 绘制水平线
    for (qreal y = rect.top() + m_gridSize; y < rect.bottom(); y += m_gridSize) {
        painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
    }

    // 绘制垂直线
    for (qreal x = rect.left() + m_gridSize; x < rect.right(); x += m_gridSize) {
        painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    }

    painter->restore();
}

void LabelEditView::drawRulers(QPainter *painter, const QRectF &rect) const
{
    if (!m_document) {
        return;
    }

    painter->save();

    // 获取视口坐标
    QPointF topLeft = mapFromScene(rect.topLeft());
    QPointF bottomRight = mapFromScene(rect.bottomRight());

    // 获取标签尺寸
    QSizeF labelSize = m_document->pageRealSize();

    // 设置标尺笔和画刷
    painter->setPen(QPen(Qt::black, 1));
    painter->setBrush(QBrush(QColor(240, 240, 240)));

    // 绘制水平标尺背景
    QRectF hRulerRect(0, 0, labelSize.width(), RULER_SIZE);
    painter->fillRect(hRulerRect, QColor(240, 240, 240));

    // 绘制垂直标尺背景
    QRectF vRulerRect(0, 0, RULER_SIZE, labelSize.height());
    painter->fillRect(vRulerRect, QColor(240, 240, 240));

    // 绘制标尺边框
    painter->drawRect(hRulerRect);
    painter->drawRect(vRulerRect);

    // 绘制标尺刻度
    painter->setPen(QPen(Qt::black, 1));

    // 水平标尺刻度
    for (int x = 0; x < labelSize.width(); x += 10) {
        int tickHeight = (x % 50 == 0) ? 10 : 5;
        painter->drawLine(QPointF(x, 0), QPointF(x, tickHeight));

        // 每50像素绘制数值
        if (x % 50 == 0 && x > 0) {
            painter->drawText(QRectF(x - 20, 10, 40, 10), Qt::AlignCenter, QString::number(x));
        }
    }

    // 垂直标尺刻度
    for (int y = 0; y < labelSize.height(); y += 10) {
        int tickWidth = (y % 50 == 0) ? 10 : 5;
        painter->drawLine(QPointF(0, y), QPointF(tickWidth, y));

        // 每50像素绘制数值
        if (y % 50 == 0 && y > 0) {
            // 旋转文本
            painter->save();
            painter->translate(10, y);
            painter->rotate(-90);
            painter->drawText(QRectF(-20, -10, 40, 20), Qt::AlignCenter, QString::number(y));
            painter->restore();
        }
    }

    // 绘制角落方块
    painter->fillRect(QRectF(0, 0, RULER_SIZE, RULER_SIZE), QColor(220, 220, 220));
    painter->drawRect(QRectF(0, 0, RULER_SIZE, RULER_SIZE));

    painter->restore();
}

void LabelEditView::showContextMenu(const QPoint &pos, const QList<LabelItem*> &items)
{
    QMenu menu;

    if (items.isEmpty()) {
        // 空白区域菜单
        QAction *pasteAction = menu.addAction(tr("粘贴"));
        pasteAction->setEnabled(QApplication::clipboard()->mimeData()->hasFormat("application/x-labelitem") ||
                               QApplication::clipboard()->mimeData()->hasText() ||
                               QApplication::clipboard()->mimeData()->hasImage());
        connect(pasteAction, &QAction::triggered, this, &LabelEditView::pasteItems);

        menu.addSeparator();

        QMenu *addMenu = menu.addMenu(tr("添加"));
        QAction *addTextAction = addMenu->addAction(tr("文本"));
        QAction *addImageAction = addMenu->addAction(tr("图像"));
        QAction *addBarcodeAction = addMenu->addAction(tr("条形码"));
        QAction *addQRCodeAction = addMenu->addAction(tr("二维码"));

        connect(addTextAction, &QAction::triggered, [this, pos]() {
            QPointF scenePos = mapToScene(mapFromGlobal(pos));
            addTextElement(scenePos);
        });

        connect(addImageAction, &QAction::triggered, [this, pos]() {
            QPointF scenePos = mapToScene(mapFromGlobal(pos));
            addImageElement(QString(), scenePos);
        });

        connect(addBarcodeAction, &QAction::triggered, [this, pos]() {
            QPointF scenePos = mapToScene(mapFromGlobal(pos));
            addBarcodeElement(scenePos);
        });

        connect(addQRCodeAction, &QAction::triggered, [this, pos]() {
            QPointF scenePos = mapToScene(mapFromGlobal(pos));
            addQRCodeElement(scenePos);
        });

        menu.addSeparator();

        QMenu *viewMenu = menu.addMenu(tr("视图"));
        QAction *gridAction = viewMenu->addAction(tr("显示网格"));
        gridAction->setCheckable(true);
        gridAction->setChecked(m_gridVisible);

        QAction *rulerAction = viewMenu->addAction(tr("显示标尺"));
        rulerAction->setCheckable(true);
        rulerAction->setChecked(m_rulersVisible);

        QAction *snapAction = viewMenu->addAction(tr("对齐到网格"));
        snapAction->setCheckable(true);
        snapAction->setChecked(m_snapToGrid);

        connect(gridAction, &QAction::toggled, [this](bool checked) {
            m_gridVisible = checked;
            viewport()->update();
        });

        connect(rulerAction, &QAction::toggled, [this](bool checked) {
            m_rulersVisible = checked;
            viewport()->update();
        });

        connect(snapAction, &QAction::toggled, [this](bool checked) {
            m_snapToGrid = checked;
            emit viewStatusChanged(m_snapToGrid ? tr("已启用网格对齐") : tr("已禁用网格对齐"));
        });

        QAction *zoomInAction = viewMenu->addAction(tr("放大"));
        QAction *zoomOutAction = viewMenu->addAction(tr("缩小"));
        QAction *zoomResetAction = viewMenu->addAction(tr("重置缩放"));
        QAction *zoomFitAction = viewMenu->addAction(tr("适应窗口"));

        connect(zoomInAction, &QAction::triggered, this, &LabelEditView::zoomIn);
        connect(zoomOutAction, &QAction::triggered, this, &LabelEditView::zoomOut);
        connect(zoomResetAction, &QAction::triggered, this, &LabelEditView::zoomReset);
        connect(zoomFitAction, &QAction::triggered, this, &LabelEditView::zoomToFit);

        QAction *selectAllAction = menu.addAction(tr("全选"));
        connect(selectAllAction, &QAction::triggered, this, &LabelEditView::selectAll);
    } else {
        // 元素菜单
        QAction *cutAction = menu.addAction(tr("剪切"));
        QAction *copyAction = menu.addAction(tr("复制"));
        QAction *pasteAction = menu.addAction(tr("粘贴"));
        QAction *deleteAction = menu.addAction(tr("删除"));

        connect(cutAction, &QAction::triggered, this, &LabelEditView::cutSelectedItems);
        connect(copyAction, &QAction::triggered, this, &LabelEditView::copySelectedItems);
        connect(pasteAction, &QAction::triggered, this, &LabelEditView::pasteItems);
        connect(deleteAction, &QAction::triggered, this, &LabelEditView::deleteSelectedItems);

        menu.addSeparator();

        if (items.size() == 1) {
            LabelItem *item = items.first();

            // 根据元素类型添加特定菜单项
            if (dynamic_cast<TextItem*>(item)) {
                QAction *editTextAction = menu.addAction(tr("编辑文本"));
                connect(editTextAction, &QAction::triggered, [this, item]() {
                    TextItem *textItem = dynamic_cast<TextItem*>(item);
                    bool ok;
                    QString newText = QInputDialog::getText(this, tr("编辑文本"),
                                                       tr("文本内容:"), QLineEdit::Normal,
                                                       textItem->text(), &ok);
                    if (ok && !newText.isEmpty()) {
                        textItem->setText(newText);
                    }
                });
            } else if (dynamic_cast<ImageItem*>(item)) {
                QAction *changeImageAction = menu.addAction(tr("更换图像"));
                connect(changeImageAction, &QAction::triggered, [this, item]() {
                    ImageItem *imageItem = dynamic_cast<ImageItem*>(item);
                    QString path = QFileDialog::getOpenFileName(this, tr("选择图像"),
                        QDir::homePath(), tr("图像文件 (*.png *.jpg *.jpeg *.bmp *.gif)"));

                    if (!path.isEmpty()) {
                        imageItem->setImagePath(path);
                    }
                });
            } else if (dynamic_cast<BarcodeItem*>(item)) {
                QAction *editBarcodeAction = menu.addAction(tr("编辑条形码"));
                connect(editBarcodeAction, &QAction::triggered, [this, item]() {
                    BarcodeItem *barcodeItem = dynamic_cast<BarcodeItem*>(item);
                    bool ok;
                    QString newData = QInputDialog::getText(this, tr("编辑条形码"),
                                                       tr("条形码数据:"), QLineEdit::Normal,
                                                       barcodeItem->data(), &ok);
                    if (ok && !newData.isEmpty()) {
                        barcodeItem->setData(newData);
                    }
                });
            } else if (dynamic_cast<QRCodeItem*>(item)) {
                QAction *editQRCodeAction = menu.addAction(tr("编辑二维码"));
                connect(editQRCodeAction, &QAction::triggered, [this, item]() {
                    QRCodeItem *qrcodeItem = dynamic_cast<QRCodeItem*>(item);
                    bool ok;
                    QString newData = QInputDialog::getText(this, tr("编辑二维码"),
                                                       tr("二维码数据:"), QLineEdit::Normal,
                                                       qrcodeItem->data(), &ok);
                    if (ok && !newData.isEmpty()) {
                        qrcodeItem->setData(newData);
                    }
                });
            }
        }

        menu.addSeparator();

        // 层级菜单
        QMenu *arrangeMenu = menu.addMenu(tr("排列"));
        QAction *bringToFrontAction = arrangeMenu->addAction(tr("置于顶层"));
        QAction *sendToBackAction = arrangeMenu->addAction(tr("置于底层"));
        QAction *bringForwardAction = arrangeMenu->addAction(tr("上移一层"));
        QAction *sendBackwardAction = arrangeMenu->addAction(tr("下移一层"));

        // 连接排列动作
        connect(bringToFrontAction, &QAction::triggered, [this, items]() {
            if (m_document && !items.isEmpty()) {
                for (LabelItem *item : items) {
                    m_document->moveItemToTop(item);
                }
            }
        });

        connect(sendToBackAction, &QAction::triggered, [this, items]() {
            if (m_document && !items.isEmpty()) {
                for (LabelItem *item : items) {
                    m_document->moveItemToBottom(item);
                }
            }
        });

        connect(bringForwardAction, &QAction::triggered, [this, items]() {
            if (m_document && !items.isEmpty()) {
                for (LabelItem *item : items) {
                    m_document->moveItemUp(item);
                }
            }
        });

        connect(sendBackwardAction, &QAction::triggered, [this, items]() {
            if (m_document && !items.isEmpty()) {
                for (LabelItem *item : items) {
                    m_document->moveItemDown(item);
                }
            }
        });

        // 锁定菜单
        QAction *lockAction = menu.addAction(tr("锁定"));
        lockAction->setCheckable(true);

        // 如果所有选中项都已锁定，则设置为选中状态
        bool allLocked = true;
        for (LabelItem *item : items) {
            if (!item->isLocked()) {
                allLocked = false;
                break;
            }
        }
        lockAction->setChecked(allLocked);

        connect(lockAction, &QAction::toggled, [this, items](bool locked) {
            for (LabelItem *item : items) {
                item->setLocked(locked);
            }
        });
    }

    // 显示菜单
    menu.exec(pos);
}

void LabelEditView::handleDroppedData(const QMimeData *mimeData, const QPointF &pos)
{
    if (!m_document) {
        return;
    }

    // 处理自定义MIME类型
    if (mimeData->hasFormat("application/x-labelitem")) {
        QList<LabelItem*> items = createItemsFromMimeData(mimeData);

        // 调整位置
        QPointF offset = pos - items.first()->position();
        for (LabelItem *item : items) {
            item->setPosition(item->position() + offset);
            m_document->addItem(item);
        }
    }
    // 处理图像
    else if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        for (const QUrl &url : urls) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                QFileInfo fileInfo(filePath);

                // 检查是否是图像文件
                QStringList imageExtensions = {"png", "jpg", "jpeg", "bmp", "gif"};
                if (imageExtensions.contains(fileInfo.suffix().toLower())) {
                    addImageElement(filePath, pos);
                }
            }
        }
    }
    else if (mimeData->hasImage()) {
        // 处理剪贴板图像
        QImage image = qvariant_cast<QImage>(mimeData->imageData());

        if (!image.isNull()) {
            // 创建临时文件保存图像
            QString tempPath = QDir::tempPath() + "/temp_image_" +
                             QString::number(QDateTime::currentMSecsSinceEpoch()) + ".png";

            if (image.save(tempPath)) {
                addImageElement(tempPath, pos);
            }
        }
    }
    // 处理文本
    else if (mimeData->hasText()) {
        QString text = mimeData->text();
        TextItem *textItem = dynamic_cast<TextItem*>(addTextElement(pos));
        if (textItem) {
            textItem->setText(text);
        }
    }
}

void LabelEditView::createSelectionRect(const QPointF &startPos, const QPointF &endPos)
{
    if (!m_selectionRect) {
        return;
    }

    QRectF rect = QRectF(startPos, endPos).normalized();
    m_selectionRect->setRect(rect);
}

void LabelEditView::updateSelectionState()
{
    // 发出选择改变信号
    emit selectionChanged();
}

QMimeData* LabelEditView::createMimeDataFromItems(const QList<LabelItem*> &items) const
{
    if (items.isEmpty()) {
        return nullptr;
    }

    QMimeData *mimeData = new QMimeData();

    // 创建JSON数组存储元素数据
    QJsonArray itemsArray;

    for (const LabelItem *item : items) {
        itemsArray.append(item->toJson());
    }

    // 创建根JSON对象
    QJsonObject rootObject;
    rootObject["items"] = itemsArray;

    // 转换为JSON文档
    QJsonDocument doc(rootObject);
    QByteArray data = doc.toJson();

    // 设置自定义MIME类型
    mimeData->setData("application/x-labelitem", data);

    // 如果只有一个文本元素，也设置文本数据
    if (items.size() == 1) {
        const TextItem *textItem = dynamic_cast<const TextItem*>(items.first());
        if (textItem) {
            mimeData->setText(textItem->text());
        }
    }

    return mimeData;
}

QList<LabelItem*> LabelEditView::createItemsFromMimeData(const QMimeData *mimeData) const
{
    QList<LabelItem*> items;

    if (!mimeData->hasFormat("application/x-labelitem")) {
        return items;
    }

    // 获取数据
    QByteArray data = mimeData->data("application/x-labelitem");

    // 解析JSON
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return items;
    }

    QJsonObject rootObject = doc.object();
    QJsonArray itemsArray = rootObject["items"].toArray();

    // 创建元素
    for (const QJsonValue &value : itemsArray) {
        QJsonObject itemObject = value.toObject();
        QString itemType = itemObject["type"].toString();

        LabelItem *item = nullptr;

        if (itemType == "text") {
            item = new TextItem();
        } else if (itemType == "image") {
            item = new ImageItem();
        } else if (itemType == "barcode") {
            item = new BarcodeItem();
        } else if (itemType == "qrcode") {
            item = new QRCodeItem();
        } else {
            continue; // 未知类型
        }

        // 设置新的ID
        itemObject["id"] = QUuid::createUuid().toString(QUuid::WithoutBraces);

        // 加载属性
        if (item->fromJson(itemObject)) {
            items.append(item);
        } else {
            delete item;
        }
    }

    return items;
}