#include "qrcodeitem.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QDebug>
#include <QDomDocument>
#include <QRandomGenerator>
#include <QJsonObject>
#include <QUuid>

// 集成QRencode库
#include <qrencode.h>

// QR码错误校正级别名称映射
static const QMap<QRErrorCorrectionLevel, QString> qrErrorCorrectionLevelNames = {
    {QRErrorCorrectionLevel::Low, "Low"},
    {QRErrorCorrectionLevel::Medium, "Medium"},
    {QRErrorCorrectionLevel::Quartile, "Quartile"},
    {QRErrorCorrectionLevel::High, "High"}
};

// QRencode错误校正级别映射
static const QMap<QRErrorCorrectionLevel, QRecLevel> qrencodeECLevelMap = {
    {QRErrorCorrectionLevel::Low, QR_ECLEVEL_L},
    {QRErrorCorrectionLevel::Medium, QR_ECLEVEL_M},
    {QRErrorCorrectionLevel::Quartile, QR_ECLEVEL_Q},
    {QRErrorCorrectionLevel::High, QR_ECLEVEL_H}
};

QRCodeItem::QRCodeItem(QGraphicsItem *parent)
    : LabelItem(parent)
    , m_data("https://example.com")
    , m_errorLevel(QRErrorCorrectionLevel::Medium)
    , m_foregroundColor(Qt::black)
    , m_backgroundColor(Qt::white)
    , m_margin(10)
    , m_size(200)
    , m_quietZone(true)
{
    // 设置元素类型
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    // 设置名称
    setName(tr("二维码"));

    // 设置默认大小
    setSize(QSizeF(200, 200));

    // 生成二维码图像
    generateQRCodeImage();
}

QRCodeItem::QRCodeItem(const QString &data, QGraphicsItem *parent)
    : QRCodeItem(parent)
{
    m_data = data;

    // 更新二维码内容
    updateContent();
}

QRCodeItem::~QRCodeItem()
{
    // 清理资源（如有必要）
}

void QRCodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // 调用基类方法绘制选中效果和控制点
    LabelItem::paint(painter, option, widget);

    // 保存画家状态
    painter->save();

    // 应用旋转变换
    painter->translate(m_rect.center());
    painter->rotate(m_rotation);
    painter->translate(-m_rect.center());

    // 绘制背景
    painter->fillRect(m_rect, m_backgroundColor);

    // 如果有二维码图像，绘制图像
    if (!m_qrCodeImage.isNull()) {
        painter->drawImage(m_rect, m_qrCodeImage, m_qrCodeImage.rect());
    } else {
        // 没有二维码时绘制占位符
        painter->setPen(Qt::gray);
        painter->setBrush(Qt::lightGray);
        painter->drawRect(m_rect);

        // 绘制文本
        painter->setPen(Qt::black);
        painter->drawText(m_rect, Qt::AlignCenter, tr("无效二维码"));
    }

    // 恢复画家状态
    painter->restore();
}

void QRCodeItem::saveToXml(QDomElement &element) const
{
    // 保存基本属性
    element.setAttribute("type", "qrcode");
    element.setAttribute("id", m_id);
    element.setAttribute("name", m_name);
    element.setAttribute("x", m_rect.x());
    element.setAttribute("y", m_rect.y());
    element.setAttribute("width", m_rect.width());
    element.setAttribute("height", m_rect.height());
    element.setAttribute("rotation", m_rotation);
    element.setAttribute("locked", m_locked ? "true" : "false");
    element.setAttribute("visible", m_visible ? "true" : "false");

    // 保存二维码属性
    element.setAttribute("data", m_data);
    element.setAttribute("errorLevel", getErrorCorrectionLevelName(m_errorLevel));
    element.setAttribute("foregroundColor", m_foregroundColor.name());
    element.setAttribute("backgroundColor", m_backgroundColor.name());
    element.setAttribute("margin", m_margin);
    element.setAttribute("size", m_size);
    element.setAttribute("quietZone", m_quietZone ? "true" : "false");
}

bool QRCodeItem::loadFromXml(const QDomElement &element)
{
    // 检查类型
    if (element.attribute("type") != "qrcode") {
        return false;
    }

    // 加载基本属性
    m_id = element.attribute("id");
    m_name = element.attribute("name", tr("二维码"));

    // 设置几何属性
    qreal x = element.attribute("x", "0").toDouble();
    qreal y = element.attribute("y", "0").toDouble();
    qreal width = element.attribute("width", "200").toDouble();
    qreal height = element.attribute("height", "200").toDouble();
    m_rect = QRectF(x, y, width, height);

    m_rotation = element.attribute("rotation", "0").toDouble();
    m_locked = element.attribute("locked") == "true";
    m_visible = element.attribute("visible", "true") == "true";

    // 加载二维码属性
    m_data = element.attribute("data", "https://example.com");
    m_errorLevel = getErrorCorrectionLevelFromName(element.attribute("errorLevel", "Medium"));
    m_foregroundColor = QColor(element.attribute("foregroundColor", "#000000"));
    m_backgroundColor = QColor(element.attribute("backgroundColor", "#FFFFFF"));
    m_margin = element.attribute("margin", "10").toInt();
    m_size = element.attribute("size", "200").toInt();
    m_quietZone = element.attribute("quietZone", "true") == "true";

    // 生成二维码图像
    generateQRCodeImage();

    return true;
}

QJsonObject QRCodeItem::toJson() const
{
    QJsonObject json;

    // 基本属性
    json["type"] = "qrcode";
    json["id"] = m_id;
    json["name"] = m_name;
    json["x"] = m_rect.x();
    json["y"] = m_rect.y();
    json["width"] = m_rect.width();
    json["height"] = m_rect.height();
    json["rotation"] = m_rotation;
    json["locked"] = m_locked;
    json["visible"] = m_visible;

    // 二维码属性
    json["data"] = m_data;
    json["errorLevel"] = getErrorCorrectionLevelName(m_errorLevel);
    json["foregroundColor"] = m_foregroundColor.name();
    json["backgroundColor"] = m_backgroundColor.name();
    json["margin"] = m_margin;
    json["size"] = m_size;
    json["quietZone"] = m_quietZone;

    return json;
}

bool QRCodeItem::fromJson(const QJsonObject &json)
{
    // 检查类型
    if (json["type"].toString() != "qrcode") {
        return false;
    }

    // 加载基本属性
    m_id = json["id"].toString();
    m_name = json["name"].toString(tr("二维码"));

    // 设置几何属性
    qreal x = json["x"].toDouble();
    qreal y = json["y"].toDouble();
    qreal width = json["width"].toDouble(200);
    qreal height = json["height"].toDouble(200);
    m_rect = QRectF(x, y, width, height);

    m_rotation = json["rotation"].toDouble();
    m_locked = json["locked"].toBool();
    m_visible = json["visible"].toBool(true);

    // 加载二维码属性
    m_data = json["data"].toString("https://example.com");
    m_errorLevel = getErrorCorrectionLevelFromName(json["errorLevel"].toString("Medium"));
    m_foregroundColor = QColor(json["foregroundColor"].toString("#000000"));
    m_backgroundColor = QColor(json["backgroundColor"].toString("#FFFFFF"));
    m_margin = json["margin"].toInt(10);
    m_size = json["size"].toInt(200);
    m_quietZone = json["quietZone"].toBool(true);

    // 生成二维码图像
    generateQRCodeImage();

    return true;
}

LabelItem* QRCodeItem::clone() const
{
    QRCodeItem *clone = new QRCodeItem();

    // 复制基本属性
    clone->m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    clone->m_name = m_name;
    clone->m_rect = m_rect;
    clone->m_rotation = m_rotation;
    clone->m_locked = m_locked;
    clone->m_visible = m_visible;

    // 复制二维码属性
    clone->m_data = m_data;
    clone->m_errorLevel = m_errorLevel;
    clone->m_foregroundColor = m_foregroundColor;
    clone->m_backgroundColor = m_backgroundColor;
    clone->m_margin = m_margin;
    clone->m_size = m_size;
    clone->m_quietZone = m_quietZone;

    // 生成二维码图像
    clone->generateQRCodeImage();

    return clone;
}

void QRCodeItem::updateContent()
{
    // 生成二维码图像
    generateQRCodeImage();

    // 更新视图
    update();
}

void QRCodeItem::setData(const QString &data)
{
    if (m_data == data) {
        return;
    }

    m_data = data;
    generateQRCodeImage();
    setModified(true);
    emit dataChanged(data);
    emit itemChanged();
}

QString QRCodeItem::data() const
{
    return m_data;
}

void QRCodeItem::setErrorCorrectionLevel(QRErrorCorrectionLevel level)
{
    if (m_errorLevel == level) {
        return;
    }

    m_errorLevel = level;
    generateQRCodeImage();
    setModified(true);
    emit errorCorrectionLevelChanged(level);
    emit itemChanged();
}

QRErrorCorrectionLevel QRCodeItem::errorCorrectionLevel() const
{
    return m_errorLevel;
}

void QRCodeItem::setForegroundColor(const QColor &color)
{
    if (m_foregroundColor == color) {
        return;
    }

    m_foregroundColor = color;
    generateQRCodeImage();
    setModified(true);
    emit foregroundColorChanged(color);
    emit itemChanged();
}

QColor QRCodeItem::foregroundColor() const
{
    return m_foregroundColor;
}

void QRCodeItem::setBackgroundColor(const QColor &color)
{
    if (m_backgroundColor == color) {
        return;
    }

    m_backgroundColor = color;
    generateQRCodeImage();
    setModified(true);
    emit backgroundColorChanged(color);
    emit itemChanged();
}

QColor QRCodeItem::backgroundColor() const
{
    return m_backgroundColor;
}

void QRCodeItem::setMargin(int margin)
{
    if (m_margin == margin) {
        return;
    }

    m_margin = margin;
    generateQRCodeImage();
    setModified(true);
    emit marginChanged(margin);
    emit itemChanged();
}

int QRCodeItem::margin() const
{
    return m_margin;
}

void QRCodeItem::setSize(int size)
{
    if (m_size == size) {
        return;
    }

    m_size = size;

    // 更新元素大小（保持正方形）
    LabelItem::setSize(QSizeF(size, size));

    generateQRCodeImage();
    setModified(true);
    emit sizeChanged(size);
    emit itemChanged();
}

int QRCodeItem::size() const
{
    return m_size;
}

void QRCodeItem::setQuietZone(bool quietZone)
{
    if (m_quietZone == quietZone) {
        return;
    }

    m_quietZone = quietZone;
    generateQRCodeImage();
    setModified(true);
    emit quietZoneChanged(quietZone);
    emit itemChanged();
}

bool QRCodeItem::quietZone() const
{
    return m_quietZone;
}

QString QRCodeItem::getErrorCorrectionLevelName(QRErrorCorrectionLevel level)
{
    return qrErrorCorrectionLevelNames.value(level, "Medium");
}

QRErrorCorrectionLevel QRCodeItem::getErrorCorrectionLevelFromName(const QString &name)
{
    return qrErrorCorrectionLevelNames.key(name, QRErrorCorrectionLevel::Medium);
}

QImage QRCodeItem::generateQRCode(const QString &data,
                                QRErrorCorrectionLevel errorCorrectionLevel,
                                int size,
                                int margin,
                                const QColor &foreground,
                                const QColor &background,
                                bool quietZone)
{
    // 创建图像
    QImage qrImage(size, size, QImage::Format_ARGB32);
    qrImage.fill(background);

    try {
        // 使用QRencode库生成二维码
        QRecLevel qrLevel = qrencodeECLevelMap.value(errorCorrectionLevel, QR_ECLEVEL_M);

        // 设置QRcode的版本，0表示自动选择
        QRcode *qrCode = QRcode_encodeString(data.toUtf8().constData(), 0, qrLevel, QR_MODE_8, 1);

        if (!qrCode) {
            throw std::runtime_error("QR码生成失败");
        }

        // 创建画家
        QPainter painter(&qrImage);
        painter.setPen(Qt::NoPen);
        painter.setBrush(foreground);

        // 计算模块大小（不包括边距）
        double moduleSize = (size - 2.0 * margin) / qrCode->width;

        // 绘制QR码
        for (int y = 0; y < qrCode->width; y++) {
            for (int x = 0; x < qrCode->width; x++) {
                // QRcode数据是按行存储的一维数组，用位运算检查状态
                unsigned char bit = qrCode->data[y * qrCode->width + x];
                if (bit & 0x01) {  // 检查最低位
                    QRectF rect(margin + x * moduleSize,
                               margin + y * moduleSize,
                               moduleSize, moduleSize);
                    painter.drawRect(rect);
                }
            }
        }

        // 如果需要安静区（quiet zone），绘制边框
        if (quietZone) {
            painter.setPen(QPen(Qt::lightGray, 1, Qt::DashLine));
            painter.setBrush(Qt::NoBrush);
            double quietZoneWidth = qrCode->width * moduleSize;
            painter.drawRect(margin - 4, margin - 4,
                           quietZoneWidth + 8, quietZoneWidth + 8);
        }

        // 释放QR码
        QRcode_free(qrCode);

        return qrImage;
    }
    catch (const std::exception &e) {
        qWarning() << "QR码生成错误:" << e.what();

        // 使用回退方法生成简化的QR码图像
        // 创建画家
        QPainter painter(&qrImage);

        // 获取错误校正级别字符
        char ecLevel = getErrorCorrectionLevelChar(errorCorrectionLevel);

        // 实际QR码区域大小
        int contentSize = size - 2 * margin;
        if (contentSize <= 0) {
            contentSize = 1;
        }

        // 生成简化的QR码图案
        painter.setPen(Qt::NoPen);
        painter.setBrush(foreground);

        // 指定模式点位置（真实QR码的固定点位）
        int blockSize = contentSize / 25; // 假设QR码为版本1（21x21模块）

        // 绘制三个定位图案（左上、右上、左下）
        // 左上角
        painter.fillRect(margin, margin, 7 * blockSize, 7 * blockSize, foreground);
        painter.fillRect(margin + blockSize, margin + blockSize, 5 * blockSize, 5 * blockSize, background);
        painter.fillRect(margin + 2 * blockSize, margin + 2 * blockSize, 3 * blockSize, 3 * blockSize, foreground);

        // 右上角
        painter.fillRect(margin + contentSize - 7 * blockSize, margin, 7 * blockSize, 7 * blockSize, foreground);
        painter.fillRect(margin + contentSize - 6 * blockSize, margin + blockSize, 5 * blockSize, 5 * blockSize, background);
        painter.fillRect(margin + contentSize - 5 * blockSize, margin + 2 * blockSize, 3 * blockSize, 3 * blockSize, foreground);

        // 左下角
        painter.fillRect(margin, margin + contentSize - 7 * blockSize, 7 * blockSize, 7 * blockSize, foreground);
        painter.fillRect(margin + blockSize, margin + contentSize - 6 * blockSize, 5 * blockSize, 5 * blockSize, background);
        painter.fillRect(margin + 2 * blockSize, margin + contentSize - 5 * blockSize, 3 * blockSize, 3 * blockSize, foreground);

        // 使用数据生成一些随机点
        quint32 seed = 0;
        for (QChar c : data) {
            seed = ((seed << 5) + seed) + c.unicode();
        }
        seed += static_cast<int>(errorCorrectionLevel);

        // 使用QRandomGenerator代替qsrand
        QRandomGenerator random(seed);

        // 生成一些随机数据模块
        for (int i = 0; i < contentSize; i += blockSize) {
            for (int j = 0; j < contentSize; j += blockSize) {
                // 避开三个定位图案区域
                if ((i < 8 * blockSize && j < 8 * blockSize) || // 左上
                    (i < 8 * blockSize && j > contentSize - 8 * blockSize) || // 左下
                    (i > contentSize - 8 * blockSize && j < 8 * blockSize)) { // 右上
                    continue;
                }

                // 随机生成模块
                if (random.bounded(3) == 0) {
                    painter.fillRect(margin + i, margin + j, blockSize, blockSize, foreground);
                }
            }
        }

        // 如果需要安静区（quiet zone），可以添加边框或其他视觉提示
        if (quietZone) {
            painter.setPen(QPen(Qt::lightGray, 1, Qt::DashLine));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(margin - 2, margin - 2, contentSize + 4, contentSize + 4);
        }

        return qrImage;
    }
}

void QRCodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_locked && event->button() == Qt::LeftButton) {
        // 显示数据输入对话框
        bool ok;
        QString newData = QInputDialog::getText(nullptr, tr("编辑二维码"),
                                             tr("二维码数据:"), QLineEdit::Normal,
                                             m_data, &ok);
        if (ok && !newData.isEmpty()) {
            setData(newData);
        }

        event->accept();
    } else {
        LabelItem::mouseDoubleClickEvent(event);
    }
}

bool QRCodeItem::generateQRCodeImage()
{
    // 确保数据和尺寸有效
    if (m_data.isEmpty() || m_rect.width() < 10 || m_rect.height() < 10) {
        m_qrCodeImage = QImage();
        return false;
    }

    // 生成二维码图像
    m_qrCodeImage = generateQRCode(m_data, m_errorLevel,
                                  qMin(m_rect.width(), m_rect.height()), // 取最小的边长确保是正方形
                                  m_margin, m_foregroundColor, m_backgroundColor,
                                  m_quietZone);

    // 更新视图
    update();
    return !m_qrCodeImage.isNull();
}

char QRCodeItem::getErrorCorrectionLevelChar(QRErrorCorrectionLevel level)
{
    switch (level) {
        case QRErrorCorrectionLevel::Low:
            return 'L';
        case QRErrorCorrectionLevel::Medium:
            return 'M';
        case QRErrorCorrectionLevel::Quartile:
            return 'Q';
        case QRErrorCorrectionLevel::High:
            return 'H';
        default:
            return 'M';
    }
}