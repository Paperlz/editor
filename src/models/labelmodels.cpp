#include "labelmodels.h"
#include "../items/labelitem.h"
#include "../items/textitem.h"
#include "../items/imageitem.h"
#include "../items/barcodeitem.h"
#include "../items/qrcodeitem.h"

#include <QGraphicsScene>
#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QPageLayout>
#include <QPrinter>
#include <QPainter>
#include <QJsonArray>
#include <QUndoStack>

// ================= LabelDocument 类实现 =================

LabelDocument::LabelDocument(QObject *parent)
    : QObject(parent)
    , m_scene(nullptr)
    , m_undoStack(nullptr)
    , m_pageSize(QPrinter::A4)
    , m_orientation(QPageLayout::Portrait)
    , m_customSize(210, 297) // A4 size in mm
    , m_dpi(300)
    , m_margins(QMarginsF(10, 10, 10, 10)) // 默认10mm边距
    , m_modified(false)
{
}

LabelDocument::~LabelDocument()
{
    // 清除所有元素
    clear();
}

void LabelDocument::addItem(LabelItem *item)
{
    if (!item) {
        return;
    }

    // 确保元素不在列表中
    if (m_items.contains(item)) {
        return;
    }

    // 添加到列表
    m_items.append(item);

    // 如果有场景，添加到场景
    if (m_scene) {
        m_scene->addItem(item);
    }

    // 连接信号
    connectItemSignals(item);

    // 设置为已修改
    setModified();

    // 发出信号
    emit itemAdded(item);
}

void LabelDocument::removeItem(LabelItem *item)
{
    if (!item) {
        return;
    }

    // 确保元素在列表中
    if (!m_items.contains(item)) {
        return;
    }

    // 从列表中移除
    m_items.removeOne(item);

    // 如果有场景，从场景中移除
    if (m_scene) {
        m_scene->removeItem(item);
    }

    // 断开信号
    disconnectItemSignals(item);

    // 设置为已修改
    setModified();

    // 发出信号
    emit itemRemoved(item);
}

QList<LabelItem*> LabelDocument::items() const
{
    return m_items;
}

LabelItem* LabelDocument::itemById(const QString &id) const
{
    for (LabelItem *item : m_items) {
        if (item->id() == id) {
            return item;
        }
    }

    return nullptr;
}

LabelItem* LabelDocument::itemByName(const QString &name) const
{
    for (LabelItem *item : m_items) {
        if (item->name() == name) {
            return item;
        }
    }

    return nullptr;
}

LabelItem* LabelDocument::itemAt(int index) const
{
    if (index >= 0 && index < m_items.size()) {
        return m_items.at(index);
    }

    return nullptr;
}

int LabelDocument::itemCount() const
{
    return m_items.size();
}

void LabelDocument::clear()
{
    // 保存元素列表副本
    QList<LabelItem*> items = m_items;

    // 清空元素列表
    m_items.clear();

    // 如果有场景，从场景中移除所有元素
    if (m_scene) {
        for (LabelItem *item : items) {
            m_scene->removeItem(item);
        }
    }

    // 断开所有元素的信号
    for (LabelItem *item : items) {
        disconnectItemSignals(item);
    }

    // 删除所有元素
    qDeleteAll(items);

    // 设置为已修改
    setModified();
}

bool LabelDocument::saveToXml(QIODevice *device) const
{
    if (!device || !device->isOpen() || !device->isWritable()) {
        qWarning() << "无法写入设备";
        return false;
    }

    // 创建XML文档
    QDomDocument doc("LabelDocument");
    QDomElement root = doc.createElement("Label");
    doc.appendChild(root);

    // 添加文档属性
    root.setAttribute("version", "1.0");
    root.setAttribute("pageSize", static_cast<int>(m_pageSize));
    root.setAttribute("orientation", static_cast<int>(m_orientation));
    root.setAttribute("customWidth", m_customSize.width());
    root.setAttribute("customHeight", m_customSize.height());
    root.setAttribute("dpi", m_dpi);
    root.setAttribute("marginLeft", m_margins.left());
    root.setAttribute("marginTop", m_margins.top());
    root.setAttribute("marginRight", m_margins.right());
    root.setAttribute("marginBottom", m_margins.bottom());

    // 保存元素
    saveItemsToXml(doc, root);

    // 写入设备
    QTextStream stream(device);
    stream << doc.toString(4); // 4空格缩进

    return true;
}

bool LabelDocument::loadFromXml(QIODevice *device)
{
    if (!device || !device->isOpen() || !device->isReadable()) {
        qWarning() << "无法读取设备";
        return false;
    }

    // 读取XML文档
    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;

    if (!doc.setContent(device, &errorMsg, &errorLine, &errorColumn)) {
        qWarning() << "解析XML失败:" << errorMsg << "行:" << errorLine << "列:" << errorColumn;
        return false;
    }

    // 获取根元素
    QDomElement root = doc.documentElement();
    if (root.tagName() != "Label") {
        qWarning() << "不是有效的标签文档";
        return false;
    }

    // 清除当前元素
    clear();

    // 加载文档属性
    m_pageSize = static_cast<QPrinter::PageSize>(root.attribute("pageSize", "0").toInt());
    m_orientation = static_cast<QPageLayout::Orientation>(root.attribute("orientation", "0").toInt());
    m_customSize.setWidth(root.attribute("customWidth", "210").toDouble());
    m_customSize.setHeight(root.attribute("customHeight", "297").toDouble());
    m_dpi = root.attribute("dpi", "300").toInt();

    qreal marginLeft = root.attribute("marginLeft", "10").toDouble();
    qreal marginTop = root.attribute("marginTop", "10").toDouble();
    qreal marginRight = root.attribute("marginRight", "10").toDouble();
    qreal marginBottom = root.attribute("marginBottom", "10").toDouble();
    m_margins = QMarginsF(marginLeft, marginTop, marginRight, marginBottom);

    // 加载元素
    if (!loadItemsFromXml(root)) {
        qWarning() << "加载元素失败";
        return false;
    }

    // 重置修改状态
    resetModified();

    return true;
}

QJsonObject LabelDocument::toJson() const
{
    QJsonObject json;

    // 添加文档属性
    json["version"] = "1.0";
    json["pageSize"] = static_cast<int>(m_pageSize);
    json["orientation"] = static_cast<int>(m_orientation);
    json["customWidth"] = m_customSize.width();
    json["customHeight"] = m_customSize.height();
    json["dpi"] = m_dpi;
    json["marginLeft"] = m_margins.left();
    json["marginTop"] = m_margins.top();
    json["marginRight"] = m_margins.right();
    json["marginBottom"] = m_margins.bottom();

    // 保存元素
    QJsonArray itemsArray;
    for (const LabelItem *item : m_items) {
        itemsArray.append(item->toJson());
    }
    json["items"] = itemsArray;

    return json;
}

bool LabelDocument::fromJson(const QJsonObject &json)
{
    // 检查版本
    QString version = json["version"].toString("1.0");
    if (version != "1.0") {
        qWarning() << "不支持的文档版本:" << version;
        return false;
    }

    // 清除当前元素
    clear();

    // 加载文档属性
    m_pageSize = static_cast<QPrinter::PageSize>(json["pageSize"].toInt(0));
    m_orientation = static_cast<QPageLayout::Orientation>(json["orientation"].toInt(0));
    m_customSize.setWidth(json["customWidth"].toDouble(210));
    m_customSize.setHeight(json["customHeight"].toDouble(297));
    m_dpi = json["dpi"].toInt(300);

    qreal marginLeft = json["marginLeft"].toDouble(10);
    qreal marginTop = json["marginTop"].toDouble(10);
    qreal marginRight = json["marginRight"].toDouble(10);
    qreal marginBottom = json["marginBottom"].toDouble(10);
    m_margins = QMarginsF(marginLeft, marginTop, marginRight, marginBottom);

    // 加载元素
    QJsonArray itemsArray = json["items"].toArray();
    for (const QJsonValue &value : itemsArray) {
        QJsonObject itemJson = value.toObject();
        QString itemType = itemJson["type"].toString();

        LabelItem *item = nullptr;

        // 根据类型创建元素
        if (itemType == "text") {
            item = new TextItem();
        } else if (itemType == "image") {
            item = new ImageItem();
        } else if (itemType == "barcode") {
            item = new BarcodeItem();
        } else if (itemType == "qrcode") {
            item = new QRCodeItem();
        } else {
            qWarning() << "未知元素类型:" << itemType;
            continue;
        }

        // 加载元素属性
        if (!item->fromJson(itemJson)) {
            qWarning() << "加载元素属性失败:" << itemType;
            delete item;
            continue;
        }

        // 添加元素
        addItem(item);
    }

    // 重置修改状态
    resetModified();

    return true;
}

void LabelDocument::render(QPainter *painter, const QRectF &rect) const
{
    if (!painter) {
        return;
    }

    // 保存画家状态
    painter->save();

    // 计算缩放因子
    QSizeF pageSize = pageRealSize();
    qreal scaleX = rect.width() / pageSize.width();
    qreal scaleY = rect.height() / pageSize.height();
    qreal scale = qMin(scaleX, scaleY);

    // 计算绘制区域
    QRectF targetRect(rect.x(), rect.y(), pageSize.width() * scale, pageSize.height() * scale);
    targetRect.moveCenter(rect.center());

    // 填充背景
    painter->fillRect(rect, Qt::white);

    // 设置变换
    painter->translate(targetRect.topLeft());
    painter->scale(scale, scale);

    // 绘制元素
    for (const LabelItem *item : m_items) {
        if (item->isVisible()) {
            // 创建临时画家用于绘制元素
            QPainter itemPainter(painter->device());
            itemPainter.setRenderHints(painter->renderHints());
            itemPainter.setTransform(painter->transform());

            // 绘制元素
            const_cast<LabelItem*>(item)->paint(&itemPainter, nullptr, nullptr);
        }
    }

    // 恢复画家状态
    painter->restore();
}

QImage LabelDocument::toImage(const QSize &size) const
{
    // 创建图像
    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::white);

    // 创建画家
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // 渲染文档
    render(&painter, QRectF(0, 0, size.width(), size.height()));

    return image;
}

void LabelDocument::setScene(QGraphicsScene *scene)
{
    if (m_scene == scene) {
        return;
    }

    // 如果有旧场景，从中移除所有元素
    if (m_scene) {
        for (LabelItem *item : m_items) {
            m_scene->removeItem(item);
        }
    }

    m_scene = scene;

    // 如果有新场景，添加所有元素
    if (m_scene) {
        for (LabelItem *item : m_items) {
            m_scene->addItem(item);
        }
    }
}

QGraphicsScene* LabelDocument::scene() const
{
    return m_scene;
}

void LabelDocument::setUndoStack(QUndoStack *undoStack)
{
    m_undoStack = undoStack;
}

QUndoStack* LabelDocument::undoStack() const
{
    return m_undoStack;
}

void LabelDocument::setPageSize(QPrinter::PageSize size)
{
    if (m_pageSize == size) {
        return;
    }

    m_pageSize = size;
    setModified();
    emit pageSizeChanged(size);
}

QPrinter::PageSize LabelDocument::pageSize() const
{
    return m_pageSize;
}

void LabelDocument::setOrientation(QPageLayout::Orientation orientation)
{
    if (m_orientation == orientation) {
        return;
    }

    m_orientation = orientation;
    setModified();
    emit orientationChanged(orientation);
}

QPageLayout::Orientation LabelDocument::orientation() const
{
    return m_orientation;
}

void LabelDocument::setCustomSize(const QSizeF &size)
{
    if (m_customSize == size) {
        return;
    }

    m_customSize = size;
    setModified();
    emit customSizeChanged(size);
}

QSizeF LabelDocument::customSize() const
{
    return m_customSize;
}

void LabelDocument::setDpi(int dpi)
{
    if (m_dpi == dpi) {
        return;
    }

    m_dpi = dpi;
    setModified();
    emit dpiChanged(dpi);
}

int LabelDocument::dpi() const
{
    return m_dpi;
}

void LabelDocument::setMargins(const QMarginsF &margins)
{
    if (m_margins == margins) {
        return;
    }

    m_margins = margins;
    setModified();
    emit marginsChanged(margins);
}

QMarginsF LabelDocument::margins() const
{
    return m_margins;
}

QSizeF LabelDocument::pageRealSize() const
{
    // 使用自定义大小
    if (m_pageSize == QPrinter::Custom) {
        return m_customSize;
    }

    // 获取标准页面大小（以毫米为单位）
    QPrinter printer;
    printer.setPageSize(m_pageSize);
    QPageLayout layout = printer.pageLayout();
    QSizeF size = layout.pageSize().size(QPageSize::Millimeter);

    // 根据方向调整
    if (m_orientation == QPageLayout::Landscape && size.width() < size.height()) {
        size.transpose();
    } else if (m_orientation == QPageLayout::Portrait && size.width() > size.height()) {
        size.transpose();
    }

    return size;
}

QRectF LabelDocument::contentRect() const
{
    QSizeF size = pageRealSize();
    return QRectF(m_margins.left(), m_margins.top(),
                 size.width() - m_margins.left() - m_margins.right(),
                 size.height() - m_margins.top() - m_margins.bottom());
}

void LabelDocument::setModified()
{
    m_modified = true;
    emit documentModified();
}

bool LabelDocument::isModified() const
{
    return m_modified;
}

void LabelDocument::resetModified()
{
    m_modified = false;
}

LabelItem* LabelDocument::createItem(int type, const QPointF &pos)
{
    LabelItem *item = nullptr;

    switch (type) {
        case LabelItem::TextType:
            item = createTextItem(QString(), pos);
            break;

        case LabelItem::ImageType:
            item = createImageItem(QString(), pos);
            break;

        case LabelItem::BarcodeType:
            item = createBarcodeItem(QString(), pos);
            break;

        case LabelItem::QRCodeType:
            item = createQRCodeItem(QString(), pos);
            break;

        default:
            qWarning() << "未知元素类型:" << type;
            break;
    }

    return item;
}

LabelItem* LabelDocument::createTextItem(const QString &text, const QPointF &pos)
{
    TextItem *item = new TextItem();

    if (!text.isEmpty()) {
        item->setText(text);
    }

    if (pos != QPointF()) {
        item->setPosition(pos);
    }

    // 添加到文档
    if (m_undoStack) {
        m_undoStack->push(new AddItemCommand(this, item));
    } else {
        addItem(item);
    }

    return item;
}

LabelItem* LabelDocument::createImageItem(const QString &imagePath, const QPointF &pos)
{
    ImageItem *item = new ImageItem();

    if (!imagePath.isEmpty()) {
        item->setImagePath(imagePath);
    }

    if (pos != QPointF()) {
        item->setPosition(pos);
    }

    // 添加到文档
    if (m_undoStack) {
        m_undoStack->push(new AddItemCommand(this, item));
    } else {
        addItem(item);
    }

    return item;
}

LabelItem* LabelDocument::createBarcodeItem(const QString &data, const QPointF &pos)
{
    BarcodeItem *item = new BarcodeItem();

    if (!data.isEmpty()) {
        item->setData(data);
    }

    if (pos != QPointF()) {
        item->setPosition(pos);
    }

    // 添加到文档
    if (m_undoStack) {
        m_undoStack->push(new AddItemCommand(this, item));
    } else {
        addItem(item);
    }

    return item;
}

LabelItem* LabelDocument::createQRCodeItem(const QString &data, const QPointF &pos)
{
    QRCodeItem *item = new QRCodeItem();

    if (!data.isEmpty()) {
        item->setData(data);
    }

    if (pos != QPointF()) {
        item->setPosition(pos);
    }

    // 添加到文档
    if (m_undoStack) {
        m_undoStack->push(new AddItemCommand(this, item));
    } else {
        addItem(item);
    }

    return item;
}

bool LabelDocument::moveItemUp(LabelItem *item)
{
    int index = m_items.indexOf(item);
    if (index < 0 || index >= m_items.size() - 1) {
        return false;
    }

    int newIndex = index + 1;

    // 使用撤销命令
    if (m_undoStack) {
        m_undoStack->push(new ReorderItemCommand(this, item, index, newIndex));
        return true;
    }

    // 直接交换
    m_items.swap(index, newIndex);

    // 更新Z值
    updateItemIndexes();

    // 设置为已修改
    setModified();

    return true;
}

bool LabelDocument::moveItemDown(LabelItem *item)
{
    int index = m_items.indexOf(item);
    if (index <= 0) {
        return false;
    }

    int newIndex = index - 1;

    // 使用撤销命令
    if (m_undoStack) {
        m_undoStack->push(new ReorderItemCommand(this, item, index, newIndex));
        return true;
    }

    // 直接交换
    m_items.swap(index, newIndex);

    // 更新Z值
    updateItemIndexes();

    // 设置为已修改
    setModified();

    return true;
}

bool LabelDocument::moveItemToTop(LabelItem *item)
{
    int index = m_items.indexOf(item);
    if (index < 0 || index == m_items.size() - 1) {
        return false;
    }

    int newIndex = m_items.size() - 1;

    // 使用撤销命令
    if (m_undoStack) {
        m_undoStack->push(new ReorderItemCommand(this, item, index, newIndex));
        return true;
    }

    // 直接移动
    m_items.removeAt(index);
    m_items.append(item);

    // 更新Z值
    updateItemIndexes();

    // 设置为已修改
    setModified();

    return true;
}

bool LabelDocument::moveItemToBottom(LabelItem *item)
{
    int index = m_items.indexOf(item);
    if (index <= 0) {
        return false;
    }

    int newIndex = 0;

    // 使用撤销命令
    if (m_undoStack) {
        m_undoStack->push(new ReorderItemCommand(this, item, index, newIndex));
        return true;
    }

    // 直接移动
    m_items.removeAt(index);
    m_items.prepend(item);

    // 更新Z值
    updateItemIndexes();

    // 设置为已修改
    setModified();

    return true;
}

LabelItem* LabelDocument::cloneItem(const LabelItem *item)
{
    if (!item) {
        return nullptr;
    }

    // 克隆元素
    LabelItem *clone = item->clone();

    // 添加到文档
    if (m_undoStack) {
        m_undoStack->push(new AddItemCommand(this, clone));
    } else {
        addItem(clone);
    }

    return clone;
}

void LabelDocument::connectItemSignals(LabelItem *item)
{
    if (!item) {
        return;
    }

    // 连接元素变化信号
    connect(item, &LabelItem::itemChanged, this, [this, item]() {
        setModified();
        emit itemChanged(item);
    });
}

void LabelDocument::disconnectItemSignals(LabelItem *item)
{
    if (!item) {
        return;
    }

    // 断开所有信号
    disconnect(item, nullptr, this, nullptr);
}

void LabelDocument::updateItemIndexes()
{
    // 更新元素的Z值
    for (int i = 0; i < m_items.size(); ++i) {
        m_items[i]->setZValue(i);
    }
}

void LabelDocument::saveItemsToXml(QDomDocument &document, QDomElement &parent) const
{
    // 创建元素列表元素
    QDomElement itemsElement = document.createElement("Items");
    parent.appendChild(itemsElement);

    // 保存每个元素
    for (const LabelItem *item : m_items) {
        QDomElement itemElement = document.createElement("Item");
        item->saveToXml(itemElement);
        itemsElement.appendChild(itemElement);
    }
}

bool LabelDocument::loadItemsFromXml(const QDomElement &parent)
{
    // 获取元素列表元素
    QDomElement itemsElement = parent.firstChildElement("Items");
    if (itemsElement.isNull()) {
        qWarning() << "找不到元素列表元素";
        return false;
    }

    // 加载每个元素
    QDomElement itemElement = itemsElement.firstChildElement("Item");
    while (!itemElement.isNull()) {
        QString itemType = itemElement.attribute("type");

        LabelItem *item = nullptr;

        // 根据类型创建元素
        if (itemType == "text") {
            item = new TextItem();
        } else if (itemType == "image") {
            item = new ImageItem();
        } else if (itemType == "barcode") {
            item = new BarcodeItem();
        } else if (itemType == "qrcode") {
            item = new QRCodeItem();
        } else {
            qWarning() << "未知元素类型:" << itemType;
            itemElement = itemElement.nextSiblingElement("Item");
            continue;
        }

        // 加载元素属性
        if (!item->loadFromXml(itemElement)) {
            qWarning() << "加载元素属性失败:" << itemType;
            delete item;
            itemElement = itemElement.nextSiblingElement("Item");
            continue;
        }

        // 添加元素
        addItem(item);

        // 下一个元素
        itemElement = itemElement.nextSiblingElement("Item");
    }

    return true;
}

// ================= 命令类实现 =================

// AddItemCommand 实现
AddItemCommand::AddItemCommand(LabelDocument *document, LabelItem *item)
    : QUndoCommand(QObject::tr("添加 %1").arg(item->name()))
    , m_document(document)
    , m_item(item)
    , m_ownsItem(true)
{
}

AddItemCommand::~AddItemCommand()
{
    // 如果命令拥有元素，释放它
    if (m_ownsItem) {
        delete m_item;
    }
}

void AddItemCommand::redo()
{
    m_document->addItem(m_item);
    m_ownsItem = false;
}

void AddItemCommand::undo()
{
    m_document->removeItem(m_item);
    m_ownsItem = true;
}

// RemoveItemCommand 实现
RemoveItemCommand::RemoveItemCommand(LabelDocument *document, LabelItem *item)
    : QUndoCommand(QObject::tr("删除 %1").arg(item->name()))
    , m_document(document)
    , m_item(item)
    , m_index(document->items().indexOf(item))
    , m_ownsItem(false)
{
}

RemoveItemCommand::~RemoveItemCommand()
{
    // 如果命令拥有元素，释放它
    if (m_ownsItem) {
        delete m_item;
    }
}

void RemoveItemCommand::redo()
{
    m_document->removeItem(m_item);
    m_ownsItem = true;
}

void RemoveItemCommand::undo()
{
    m_document->addItem(m_item);
    m_ownsItem = false;
}

// ReorderItemCommand 实现
ReorderItemCommand::ReorderItemCommand(LabelDocument *document, LabelItem *item, int oldIndex, int newIndex)
    : QUndoCommand(QObject::tr("移动 %1").arg(item->name()))
    , m_document(document)
    , m_item(item)
    , m_oldIndex(oldIndex)
    , m_newIndex(newIndex)
{
}

void ReorderItemCommand::redo()
{
    // 从旧位置移除
    m_document->items().removeAt(m_oldIndex);

    // 插入到新位置
    m_document->items().insert(m_newIndex, m_item);

    // 更新Z值
    m_document->updateItemIndexes();

    // 设置为已修改
    m_document->setModified();
}

void ReorderItemCommand::undo()
{
    // 从新位置移除
    m_document->items().removeAt(m_newIndex);

    // 插入到旧位置
    m_document->items().insert(m_oldIndex, m_item);

    // 更新Z值
    m_document->updateItemIndexes();

    // 设置为已修改
    m_document->setModified();
}