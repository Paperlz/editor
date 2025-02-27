#include "imageitem.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QDomDocument>
#include <QJsonObject>
#include <QBuffer>
#include <QFileInfo>
#include <QImageReader>
#include <QApplication>
#include <QImageWriter>
#include <QClipboard>
#include <QMimeData>
#include <QUuid>

ImageItem::ImageItem(QGraphicsItem *parent)
    : LabelItem(parent)
    , m_keepAspectRatio(true)
    , m_borderWidth(0)
    , m_borderColor(Qt::black)
    , m_opacity(1.0)
    , m_grayScale(false)
    , m_brightness(0)
    , m_contrast(0)
{
    // 设置元素类型
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    // 设置名称
    setName(tr("图像"));

    // 设置默认大小
    setSize(QSizeF(100, 100));
}

ImageItem::ImageItem(const QString &imagePath, QGraphicsItem *parent)
    : ImageItem(parent)
{
    setImagePath(imagePath);
}

ImageItem::~ImageItem()
{
    // 清理资源（如有必要）
}

void ImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // 调用基类方法绘制选中效果和控制点
    LabelItem::paint(painter, option, widget);

    // 保存画家状态
    painter->save();

    // 应用旋转变换
    painter->translate(m_rect.center());
    painter->rotate(m_rotation);
    painter->translate(-m_rect.center());

    // 设置不透明度
    painter->setOpacity(m_opacity);

    // 如果有图像，绘制图像
    if (!m_pixmap.isNull()) {
        painter->drawPixmap(m_rect, m_pixmap, m_pixmap.rect());
    } else {
        // 没有图像时绘制占位符
        painter->setPen(Qt::gray);
        painter->setBrush(Qt::lightGray);
        painter->drawRect(m_rect);
        painter->drawLine(m_rect.topLeft(), m_rect.bottomRight());
        painter->drawLine(m_rect.topRight(), m_rect.bottomLeft());

        // 绘制文本
        painter->setPen(Qt::black);
        painter->drawText(m_rect, Qt::AlignCenter, tr("无图像"));
    }

    // 绘制边框
    if (m_borderWidth > 0) {
        painter->setPen(QPen(m_borderColor, m_borderWidth));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(m_rect);
    }

    // 恢复画家状态
    painter->restore();
}

void ImageItem::saveToXml(QDomElement &element) const
{
    // 保存基本属性
    element.setAttribute("type", "image");
    element.setAttribute("id", m_id);
    element.setAttribute("name", m_name);
    element.setAttribute("x", m_rect.x());
    element.setAttribute("y", m_rect.y());
    element.setAttribute("width", m_rect.width());
    element.setAttribute("height", m_rect.height());
    element.setAttribute("rotation", m_rotation);
    element.setAttribute("locked", m_locked ? "true" : "false");
    element.setAttribute("visible", m_visible ? "true" : "false");

    // 保存图像属性
    element.setAttribute("imagePath", m_imagePath);
    element.setAttribute("keepAspectRatio", m_keepAspectRatio ? "true" : "false");
    element.setAttribute("borderWidth", m_borderWidth);
    element.setAttribute("borderColor", m_borderColor.name());
    element.setAttribute("opacity", m_opacity);
    element.setAttribute("grayScale", m_grayScale ? "true" : "false");
    element.setAttribute("brightness", m_brightness);
    element.setAttribute("contrast", m_contrast);

    // 如果没有图像路径（可能是直接设置的图像数据），保存图像数据
    if (m_imagePath.isEmpty() && !m_originalImage.isNull()) {
        QByteArray imageData;
        QBuffer buffer(&imageData);
        buffer.open(QIODevice::WriteOnly);
        m_originalImage.save(&buffer, "PNG");
        buffer.close();

        QDomElement dataElement = element.ownerDocument().createElement("imageData");
        QDomText dataText = element.ownerDocument().createTextNode(QString(imageData.toBase64()));
        dataElement.appendChild(dataText);
        element.appendChild(dataElement);
    }
}

bool ImageItem::loadFromXml(const QDomElement &element)
{
    // 检查类型
    if (element.attribute("type") != "image") {
        return false;
    }

    // 加载基本属性
    m_id = element.attribute("id");
    m_name = element.attribute("name", tr("图像"));

    // 设置几何属性
    qreal x = element.attribute("x", "0").toDouble();
    qreal y = element.attribute("y", "0").toDouble();
    qreal width = element.attribute("width", "100").toDouble();
    qreal height = element.attribute("height", "100").toDouble();
    m_rect = QRectF(x, y, width, height);

    m_rotation = element.attribute("rotation", "0").toDouble();
    m_locked = element.attribute("locked") == "true";
    m_visible = element.attribute("visible", "true") == "true";

    // 加载图像属性
    m_imagePath = element.attribute("imagePath");
    m_keepAspectRatio = element.attribute("keepAspectRatio", "true") == "true";
    m_borderWidth = element.attribute("borderWidth", "0").toInt();
    m_borderColor = QColor(element.attribute("borderColor", "#000000"));
    m_opacity = element.attribute("opacity", "1.0").toDouble();
    m_grayScale = element.attribute("grayScale") == "true";
    m_brightness = element.attribute("brightness", "0").toInt();
    m_contrast = element.attribute("contrast", "0").toInt();

    // 尝试加载图像
    bool imageLoaded = false;

    if (!m_imagePath.isEmpty()) {
        // 从文件加载图像
        imageLoaded = setImagePath(m_imagePath);
    }

    if (!imageLoaded) {
        // 尝试从数据加载图像
        QDomElement dataElement = element.firstChildElement("imageData");
        if (!dataElement.isNull()) {
            QByteArray imageData = QByteArray::fromBase64(dataElement.text().toLatin1());
            QImage image;
            if (image.loadFromData(imageData)) {
                setImage(image);
                imageLoaded = true;
            }
        }
    }

    // 更新内容
    updateContent();

    return true;
}

QJsonObject ImageItem::toJson() const
{
    QJsonObject json;

    // 基本属性
    json["type"] = "image";
    json["id"] = m_id;
    json["name"] = m_name;
    json["x"] = m_rect.x();
    json["y"] = m_rect.y();
    json["width"] = m_rect.width();
    json["height"] = m_rect.height();
    json["rotation"] = m_rotation;
    json["locked"] = m_locked;
    json["visible"] = m_visible;

    // 图像属性
    json["imagePath"] = m_imagePath;
    json["keepAspectRatio"] = m_keepAspectRatio;
    json["borderWidth"] = m_borderWidth;
    json["borderColor"] = m_borderColor.name();
    json["opacity"] = m_opacity;
    json["grayScale"] = m_grayScale;
    json["brightness"] = m_brightness;
    json["contrast"] = m_contrast;

    // 如果没有图像路径，保存图像数据
    if (m_imagePath.isEmpty() && !m_originalImage.isNull()) {
        QByteArray imageData;
        QBuffer buffer(&imageData);
        buffer.open(QIODevice::WriteOnly);
        m_originalImage.save(&buffer, "PNG");
        buffer.close();

        json["imageData"] = QString(imageData.toBase64());
    }

    return json;
}

bool ImageItem::fromJson(const QJsonObject &json)
{
    // 检查类型
    if (json["type"].toString() != "image") {
        return false;
    }

    // 加载基本属性
    m_id = json["id"].toString();
    m_name = json["name"].toString(tr("图像"));

    // 设置几何属性
    qreal x = json["x"].toDouble();
    qreal y = json["y"].toDouble();
    qreal width = json["width"].toDouble(100);
    qreal height = json["height"].toDouble(100);
    m_rect = QRectF(x, y, width, height);

    m_rotation = json["rotation"].toDouble();
    m_locked = json["locked"].toBool();
    m_visible = json["visible"].toBool(true);

    // 加载图像属性
    m_imagePath = json["imagePath"].toString();
    m_keepAspectRatio = json["keepAspectRatio"].toBool(true);
    m_borderWidth = json["borderWidth"].toInt(0);
    m_borderColor = QColor(json["borderColor"].toString("#000000"));
    m_opacity = json["opacity"].toDouble(1.0);
    m_grayScale = json["grayScale"].toBool(false);
    m_brightness = json["brightness"].toInt(0);
    m_contrast = json["contrast"].toInt(0);

    // 尝试加载图像
    bool imageLoaded = false;

    if (!m_imagePath.isEmpty()) {
        // 从文件加载图像
        imageLoaded = setImagePath(m_imagePath);
    }

    if (!imageLoaded) {
        // 尝试从数据加载图像
        QString imageDataBase64 = json["imageData"].toString();
        if (!imageDataBase64.isEmpty()) {
            QByteArray imageData = QByteArray::fromBase64(imageDataBase64.toLatin1());
            QImage image;
            if (image.loadFromData(imageData)) {
                setImage(image);
                imageLoaded = true;
            }
        }
    }

    // 更新内容
    updateContent();

    return true;
}

LabelItem* ImageItem::clone() const
{
    ImageItem *clone = new ImageItem();

    // 复制基本属性
    clone->m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    clone->m_name = m_name;
    clone->m_rect = m_rect;
    clone->m_rotation = m_rotation;
    clone->m_locked = m_locked;
    clone->m_visible = m_visible;

    // 复制图像属性
    clone->m_imagePath = m_imagePath;
    clone->m_originalImage = m_originalImage;
    clone->m_processedImage = m_processedImage;
    clone->m_pixmap = m_pixmap;
    clone->m_keepAspectRatio = m_keepAspectRatio;
    clone->m_borderWidth = m_borderWidth;
    clone->m_borderColor = m_borderColor;
    clone->m_opacity = m_opacity;
    clone->m_grayScale = m_grayScale;
    clone->m_brightness = m_brightness;
    clone->m_contrast = m_contrast;

    // 更新内容
    clone->updateContent();

    return clone;
}

void ImageItem::updateContent()
{
    // 应用图像效果
    applyEffects();

    // 更新视图
    update();
}

bool ImageItem::setImagePath(const QString &path)
{
    if (path.isEmpty()) {
        return false;
    }

    // 加载图像
    QImageReader reader(path);
    QImage img = reader.read();

    if (img.isNull()) {
        qWarning() << "无法加载图像:" << path << ", 错误:" << reader.errorString();
        return false;
    }

    // 保存图像路径
    m_imagePath = path;

    // 设置图像
    setImage(img);

    // 如果是新加载的图像，可能需要调整元素大小
    if (m_rect.width() <= 1 || m_rect.height() <= 1) {
        // 计算适当的大小，确保不会太大
        QSizeF size = img.size();
        if (size.width() > 300 || size.height() > 300) {
            size.scale(300, 300, Qt::KeepAspectRatio);
        }
        setSize(size);
    }

    emit imagePathChanged(path);
    return true;
}

QString ImageItem::imagePath() const
{
    return m_imagePath;
}

bool ImageItem::setImage(const QImage &image)
{
    if (image.isNull()) {
        return false;
    }

    // 保存原始图像
    m_originalImage = image;

    // 应用效果
    applyEffects();

    // 标记为已修改
    setModified(true);
    emit itemChanged();

    return true;
}

QImage ImageItem::image() const
{
    return m_processedImage.isNull() ? m_originalImage : m_processedImage;
}

void ImageItem::setKeepAspectRatio(bool keep)
{
    if (m_keepAspectRatio == keep) {
        return;
    }

    m_keepAspectRatio = keep;

    // 如果启用了保持宽高比，可能需要调整大小
    if (keep && !m_originalImage.isNull()) {
        QSizeF currentSize = size();
        QSizeF imgSize = m_originalImage.size();
        qreal ratio = imgSize.width() / imgSize.height();

        // 根据宽高比调整大小
        if (currentSize.width() / currentSize.height() != ratio) {
            // 以当前宽度为基准调整高度
            QSizeF newSize(currentSize.width(), currentSize.width() / ratio);
            setSize(newSize);
        }
    }

    setModified(true);
    emit keepAspectRatioChanged(keep);
    emit itemChanged();
}

bool ImageItem::keepAspectRatio() const
{
    return m_keepAspectRatio;
}

void ImageItem::setBorderWidth(int width)
{
    if (m_borderWidth == width) {
        return;
    }

    m_borderWidth = width;
    update();
    setModified(true);
    emit borderWidthChanged(width);
    emit itemChanged();
}

int ImageItem::borderWidth() const
{
    return m_borderWidth;
}

void ImageItem::setBorderColor(const QColor &color)
{
    if (m_borderColor == color) {
        return;
    }

    m_borderColor = color;
    update();
    setModified(true);
    emit borderColorChanged(color);
    emit itemChanged();
}

QColor ImageItem::borderColor() const
{
    return m_borderColor;
}

void ImageItem::setOpacity(qreal opacity)
{
    if (qFuzzyCompare(m_opacity, opacity)) {
        return;
    }

    // 确保透明度在有效范围内
    m_opacity = qBound(0.0, opacity, 1.0);
    update();
    setModified(true);
    emit opacityChanged(opacity);
    emit itemChanged();
}

qreal ImageItem::opacity() const
{
    return m_opacity;
}

void ImageItem::setGrayScale(bool gray)
{
    if (m_grayScale == gray) {
        return;
    }

    m_grayScale = gray;
    applyEffects();
    setModified(true);
    emit grayScaleChanged(gray);
    emit itemChanged();
}

bool ImageItem::grayScale() const
{
    return m_grayScale;
}

void ImageItem::cropImage(const QRectF &rect)
{
    if (m_originalImage.isNull() || !rect.isValid()) {
        return;
    }

    // 保存旧图像
    QImage oldImage = m_originalImage;

    // 计算裁剪矩形
    QRect cropRect = rect.toRect();
    cropRect = cropRect.intersected(m_originalImage.rect());

    if (cropRect.isEmpty()) {
        return;
    }

    // 裁剪图像
    QImage croppedImage = m_originalImage.copy(cropRect);

    if (croppedImage.isNull()) {
        return;
    }

    // 设置新图像
    setImage(croppedImage);

    // 发出信号
    emit imageProcessed();
}

void ImageItem::rotateImage(qreal angle)
{
    if (m_originalImage.isNull() || qFuzzyCompare(angle, 0.0)) {
        return;
    }

    // 保存旧图像
    QImage oldImage = m_originalImage;

    // 创建变换矩阵
    QTransform transform;
    transform.rotate(angle);

    // 应用变换
    QImage rotatedImage = m_originalImage.transformed(transform, Qt::SmoothTransformation);

    if (rotatedImage.isNull()) {
        return;
    }

    // 设置新图像
    setImage(rotatedImage);

    // 发出信号
    emit imageProcessed();
}

void ImageItem::flipImage(bool horizontal)
{
    if (m_originalImage.isNull()) {
        return;
    }

    // 保存旧图像
    QImage oldImage = m_originalImage;

    // 翻转图像
    QImage flippedImage = m_originalImage.mirrored(horizontal, !horizontal);

    if (flippedImage.isNull()) {
        return;
    }

    // 设置新图像
    setImage(flippedImage);

    // 发出信号
    emit imageProcessed();
}

void ImageItem::adjustBrightness(int brightness)
{
    if (m_brightness == brightness) {
        return;
    }

    // 确保亮度在有效范围内
    m_brightness = qBound(-100, brightness, 100);
    applyEffects();
    setModified(true);
    emit itemChanged();
}

void ImageItem::adjustContrast(int contrast)
{
    if (m_contrast == contrast) {
        return;
    }

    // 确保对比度在有效范围内
    m_contrast = qBound(-100, contrast, 100);
    applyEffects();
    setModified(true);
    emit itemChanged();
}

void ImageItem::resetImage()
{
    if (m_originalImage.isNull()) {
        return;
    }

    // 重置效果
    m_grayScale = false;
    m_brightness = 0;
    m_contrast = 0;

    // 更新图像
    m_processedImage = QImage();
    m_pixmap = QPixmap::fromImage(m_originalImage);

    update();
    setModified(true);
    emit grayScaleChanged(false);
    emit itemChanged();
}

void ImageItem::resize(qreal width, qreal height)
{
    if (m_keepAspectRatio && !m_originalImage.isNull()) {
        // 保持宽高比
        QSizeF imgSize = m_originalImage.size();
        qreal ratio = imgSize.width() / imgSize.height();

        if (width > 0 && height > 0) {
            // 根据较小的比例调整大小
            qreal widthRatio = width / imgSize.width();
            qreal heightRatio = height / imgSize.height();

            if (widthRatio < heightRatio) {
                height = width / ratio;
            } else {
                width = height * ratio;
            }
        } else if (width > 0) {
            height = width / ratio;
        } else if (height > 0) {
            width = height * ratio;
        }
    }

    // 调用基类方法设置大小
    LabelItem::resize(width, height);
}

void ImageItem::applyEffects()
{
    if (m_originalImage.isNull()) {
        return;
    }

    // 如果没有任何效果，直接使用原始图像
    if (!m_grayScale && m_brightness == 0 && m_contrast == 0) {
        m_processedImage = QImage();
        m_pixmap = QPixmap::fromImage(m_originalImage);
        update();
        return;
    }

    // 创建处理后的图像副本
    m_processedImage = m_originalImage.copy();

    // 应用灰度效果
    if (m_grayScale) {
        for (int y = 0; y < m_processedImage.height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb*>(m_processedImage.scanLine(y));
            for (int x = 0; x < m_processedImage.width(); ++x) {
                int gray = qGray(line[x]);
                line[x] = qRgb(gray, gray, gray);
            }
        }
    }

    // 应用亮度和对比度调整
    if (m_brightness != 0 || m_contrast != 0) {
        // 亮度和对比度因子
        qreal brightnessF = 1.0 + m_brightness / 100.0;
        qreal contrastF = 1.0 + m_contrast / 100.0;

        for (int y = 0; y < m_processedImage.height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb*>(m_processedImage.scanLine(y));
            for (int x = 0; x < m_processedImage.width(); ++x) {
                int r = qRed(line[x]);
                int g = qGreen(line[x]);
                int b = qBlue(line[x]);

                // 应用亮度
                if (m_brightness != 0) {
                    r = qBound(0, static_cast<int>(r * brightnessF), 255);
                    g = qBound(0, static_cast<int>(g * brightnessF), 255);
                    b = qBound(0, static_cast<int>(b * brightnessF), 255);
                }

                // 应用对比度
                if (m_contrast != 0) {
                    r = qBound(0, static_cast<int>(((r / 255.0 - 0.5) * contrastF + 0.5) * 255), 255);
                    g = qBound(0, static_cast<int>(((g / 255.0 - 0.5) * contrastF + 0.5) * 255), 255);
                    b = qBound(0, static_cast<int>(((b / 255.0 - 0.5) * contrastF + 0.5) * 255), 255);
                }

                line[x] = qRgba(r, g, b, qAlpha(line[x]));
            }
        }
    }

    // 更新像素图
    m_pixmap = QPixmap::fromImage(m_processedImage);

    // 更新视图
    update();

    // 发出信号
    emit imageProcessed();
}

void ImageItem::saveOriginalImage()
{
    // 目前不需要实现，因为原始图像已在设置图像时保存
}

// ============ AdjustImageCommand 实现 ============

AdjustImageCommand::AdjustImageCommand(ImageItem *item, const QImage &oldImage,
                                     const QImage &newImage, const QString &name)
    : QUndoCommand(name)
    , m_item(item)
    , m_oldImage(oldImage)
    , m_newImage(newImage)
{
}

void AdjustImageCommand::redo()
{
    m_item->setImage(m_newImage);
}

void AdjustImageCommand::undo()
{
    m_item->setImage(m_oldImage);
}