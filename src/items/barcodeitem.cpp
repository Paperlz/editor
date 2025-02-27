#include "barcodeitem.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QDebug>
#include <QDomDocument>
#include <QJsonObject>
#include <QUuid>
#include <QFontMetrics>

// 条形码类型名称映射
static const QMap<BarcodeType, QString> barcodeTypeNames = {
    {BarcodeType::Code128, "Code 128"},
    {BarcodeType::Code39, "Code 39"},
    {BarcodeType::Code93, "Code 93"},
    {BarcodeType::EAN8, "EAN-8"},
    {BarcodeType::EAN13, "EAN-13"},
    {BarcodeType::UPC_A, "UPC-A"},
    {BarcodeType::UPC_E, "UPC-E"},
    {BarcodeType::MSI, "MSI"},
    {BarcodeType::Interleaved2of5, "Interleaved 2 of 5"},
    {BarcodeType::ITF14, "ITF-14"},
    {BarcodeType::Codabar, "Codabar"}
};

// Code 128 编码表
static const QList<QList<int>> code128Patterns = {
    // Code 128 A 开始符
    {2, 1, 1, 4, 1, 2},   // START A
    // Code 128 B 开始符
    {2, 1, 1, 2, 1, 4},   // START B
    // Code 128 C 开始符
    {2, 1, 1, 2, 3, 2},   // START C
    // 停止符
    {2, 3, 3, 1, 1, 1, 2} // STOP
};

// Code 39 字符集和编码
static const QMap<QChar, QString> code39Patterns = {
    {'0', "bwbwwwbwwb"}, {'1', "wwbwbwbwwb"}, {'2', "bwbwbwbwww"},
    {'3', "wwbwbwbwww"}, {'4', "bwwwbwbwwb"}, {'5', "wwwwbwbwwb"},
    {'6', "bwwwbwbwww"}, {'7', "bwwwwwbwwb"}, {'8', "wwwwwwbwwb"},
    {'9', "bwwwwwbwww"}, {'A', "wwbwwwbwwb"}, {'B', "bwbwwwbwww"},
    {'C', "wwbwwwbwww"}, {'D', "bwwwbwwwwb"}, {'E', "wwwwbwwwwb"},
    {'F', "bwwwbwwwww"}, {'G', "bwwwwwbwwb"}, {'H', "wwwwwwbwwb"},
    {'I', "bwwwwwbwww"}, {'J', "wwwwbwwwww"}, {'K', "wwbwwwwwbw"},
    {'L', "bwbwwwwwbw"}, {'M', "wwbwwwwwbw"}, {'N', "bwwwbwwwbw"},
    {'O', "wwwwbwwwbw"}, {'P', "bwwwbwwwbw"}, {'Q', "bwwwwwbwbw"},
    {'R', "wwwwwwbwbw"}, {'S', "bwwwwwbwbw"}, {'T', "wwwwbwwwbw"},
    {'U', "wwbwwwbwbw"}, {'V', "bwbwwwbwbw"}, {'W', "wwbwwwbwbw"},
    {'X', "bwwwbwbwbw"}, {'Y', "wwwwbwbwbw"}, {'Z', "bwwwbwbwbw"},
    {'-', "bwwwwwbwbw"}, {'.', "wwwwwwbwbw"}, {' ', "bwwwbwbwbw"},
    {'*', "bwwwbwbwb"}   // 开始/结束符
};

BarcodeItem::BarcodeItem(QGraphicsItem *parent)
    : LabelItem(parent)
    , m_data("12345678")
    , m_type(BarcodeType::Code128)
    , m_foregroundColor(Qt::black)
    , m_backgroundColor(Qt::white)
    , m_showText(true)
    , m_textFont(QFont("Arial", 8))
    , m_margin(10)
    , m_includeChecksum(true)
{
    // 设置元素类型
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    // 设置名称
    setName(tr("条形码"));

    // 设置默认大小
    setSize(QSizeF(200, 100));

    // 生成条形码图像
    generateBarcodeImage();
}

BarcodeItem::BarcodeItem(const QString &data, BarcodeType type, QGraphicsItem *parent)
    : BarcodeItem(parent)
{
    m_data = data;
    m_type = type;

    // 更新条形码内容
    updateContent();
}

BarcodeItem::~BarcodeItem()
{
    // 清理资源（如有必要）
}

void BarcodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

    // 如果有条形码图像，绘制图像
    if (!m_barcodeImage.isNull()) {
        painter->drawImage(m_rect, m_barcodeImage, m_barcodeImage.rect());
    } else {
        // 没有条形码时绘制占位符
        painter->setPen(Qt::gray);
        painter->setBrush(Qt::lightGray);
        painter->drawRect(m_rect);

        // 绘制文本
        painter->setPen(Qt::black);
        painter->drawText(m_rect, Qt::AlignCenter, tr("无效条形码"));
    }

    // 恢复画家状态
    painter->restore();
}

void BarcodeItem::saveToXml(QDomElement &element) const
{
    // 保存基本属性
    element.setAttribute("type", "barcode");
    element.setAttribute("id", m_id);
    element.setAttribute("name", m_name);
    element.setAttribute("x", m_rect.x());
    element.setAttribute("y", m_rect.y());
    element.setAttribute("width", m_rect.width());
    element.setAttribute("height", m_rect.height());
    element.setAttribute("rotation", m_rotation);
    element.setAttribute("locked", m_locked ? "true" : "false");
    element.setAttribute("visible", m_visible ? "true" : "false");

    // 保存条形码属性
    element.setAttribute("data", m_data);
    element.setAttribute("barcodeType", getTypeName(m_type));
    element.setAttribute("foregroundColor", m_foregroundColor.name());
    element.setAttribute("backgroundColor", m_backgroundColor.name());
    element.setAttribute("showText", m_showText ? "true" : "false");
    element.setAttribute("margin", m_margin);
    element.setAttribute("includeChecksum", m_includeChecksum ? "true" : "false");

    // 保存字体属性
    QDomElement fontElement = element.ownerDocument().createElement("font");
    fontElement.setAttribute("family", m_textFont.family());
    fontElement.setAttribute("pointSize", m_textFont.pointSize());
    fontElement.setAttribute("bold", m_textFont.bold() ? "true" : "false");
    fontElement.setAttribute("italic", m_textFont.italic() ? "true" : "false");
    element.appendChild(fontElement);
}

bool BarcodeItem::loadFromXml(const QDomElement &element)
{
    // 检查类型
    if (element.attribute("type") != "barcode") {
        return false;
    }

    // 加载基本属性
    m_id = element.attribute("id");
    m_name = element.attribute("name", tr("条形码"));

    // 设置几何属性
    qreal x = element.attribute("x", "0").toDouble();
    qreal y = element.attribute("y", "0").toDouble();
    qreal width = element.attribute("width", "200").toDouble();
    qreal height = element.attribute("height", "100").toDouble();
    m_rect = QRectF(x, y, width, height);

    m_rotation = element.attribute("rotation", "0").toDouble();
    m_locked = element.attribute("locked") == "true";
    m_visible = element.attribute("visible", "true") == "true";

    // 加载条形码属性
    m_data = element.attribute("data", "12345678");
    m_type = getTypeFromName(element.attribute("barcodeType", "Code 128"));
    m_foregroundColor = QColor(element.attribute("foregroundColor", "#000000"));
    m_backgroundColor = QColor(element.attribute("backgroundColor", "#FFFFFF"));
    m_showText = element.attribute("showText", "true") == "true";
    m_margin = element.attribute("margin", "10").toInt();
    m_includeChecksum = element.attribute("includeChecksum", "true") == "true";

    // 加载字体属性
    QDomElement fontElement = element.firstChildElement("font");
    if (!fontElement.isNull()) {
        QString family = fontElement.attribute("family", "Arial");
        int pointSize = fontElement.attribute("pointSize", "8").toInt();
        bool bold = fontElement.attribute("bold") == "true";
        bool italic = fontElement.attribute("italic") == "true";

        m_textFont = QFont(family, pointSize);
        m_textFont.setBold(bold);
        m_textFont.setItalic(italic);
    }

    // 生成条形码图像
    generateBarcodeImage();

    return true;
}

QJsonObject BarcodeItem::toJson() const
{
    QJsonObject json;

    // 基本属性
    json["type"] = "barcode";
    json["id"] = m_id;
    json["name"] = m_name;
    json["x"] = m_rect.x();
    json["y"] = m_rect.y();
    json["width"] = m_rect.width();
    json["height"] = m_rect.height();
    json["rotation"] = m_rotation;
    json["locked"] = m_locked;
    json["visible"] = m_visible;

    // 条形码属性
    json["data"] = m_data;
    json["barcodeType"] = getTypeName(m_type);
    json["foregroundColor"] = m_foregroundColor.name();
    json["backgroundColor"] = m_backgroundColor.name();
    json["showText"] = m_showText;
    json["margin"] = m_margin;
    json["includeChecksum"] = m_includeChecksum;

    // 字体属性
    QJsonObject fontJson;
    fontJson["family"] = m_textFont.family();
    fontJson["pointSize"] = m_textFont.pointSize();
    fontJson["bold"] = m_textFont.bold();
    fontJson["italic"] = m_textFont.italic();
    json["font"] = fontJson;

    return json;
}

bool BarcodeItem::fromJson(const QJsonObject &json)
{
    // 检查类型
    if (json["type"].toString() != "barcode") {
        return false;
    }

    // 加载基本属性
    m_id = json["id"].toString();
    m_name = json["name"].toString(tr("条形码"));

    // 设置几何属性
    qreal x = json["x"].toDouble();
    qreal y = json["y"].toDouble();
    qreal width = json["width"].toDouble(200);
    qreal height = json["height"].toDouble(100);
    m_rect = QRectF(x, y, width, height);

    m_rotation = json["rotation"].toDouble();
    m_locked = json["locked"].toBool();
    m_visible = json["visible"].toBool(true);

    // 加载条形码属性
    m_data = json["data"].toString("12345678");
    m_type = getTypeFromName(json["barcodeType"].toString("Code 128"));
    m_foregroundColor = QColor(json["foregroundColor"].toString("#000000"));
    m_backgroundColor = QColor(json["backgroundColor"].toString("#FFFFFF"));
    m_showText = json["showText"].toBool(true);
    m_margin = json["margin"].toInt(10);
    m_includeChecksum = json["includeChecksum"].toBool(true);

    // 加载字体属性
    QJsonObject fontJson = json["font"].toObject();
    if (!fontJson.isEmpty()) {
        QString family = fontJson["family"].toString("Arial");
        int pointSize = fontJson["pointSize"].toInt(8);
        bool bold = fontJson["bold"].toBool();
        bool italic = fontJson["italic"].toBool();

        m_textFont = QFont(family, pointSize);
        m_textFont.setBold(bold);
        m_textFont.setItalic(italic);
    }

    // 生成条形码图像
    generateBarcodeImage();

    return true;
}

LabelItem* BarcodeItem::clone() const
{
    BarcodeItem *clone = new BarcodeItem();

    // 复制基本属性
    clone->m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    clone->m_name = m_name;
    clone->m_rect = m_rect;
    clone->m_rotation = m_rotation;
    clone->m_locked = m_locked;
    clone->m_visible = m_visible;

    // 复制条形码属性
    clone->m_data = m_data;
    clone->m_type = m_type;
    clone->m_foregroundColor = m_foregroundColor;
    clone->m_backgroundColor = m_backgroundColor;
    clone->m_showText = m_showText;
    clone->m_textFont = m_textFont;
    clone->m_margin = m_margin;
    clone->m_includeChecksum = m_includeChecksum;

    // 生成条形码图像
    clone->generateBarcodeImage();

    return clone;
}

void BarcodeItem::updateContent()
{
    // 生成条形码图像
    generateBarcodeImage();

    // 更新视图
    update();
}

void BarcodeItem::setData(const QString &data)
{
    if (m_data == data) {
        return;
    }

    // 验证数据有效性
    if (!validateData(data, m_type)) {
        qWarning() << "无效的条形码数据:" << data << "对于类型:" << getTypeName(m_type);
        return;
    }

    m_data = data;
    generateBarcodeImage();
    setModified(true);
    emit dataChanged(data);
    emit itemChanged();
}

QString BarcodeItem::data() const
{
    return m_data;
}

void BarcodeItem::setType(BarcodeType type)
{
    if (m_type == type) {
        return;
    }

    m_type = type;

    // 验证当前数据对于新类型是否有效
    if (!validateData(m_data, m_type)) {
        // 如果无效，设置默认数据
        switch (m_type) {
            case BarcodeType::EAN8:
                m_data = "1234567";
                break;
            case BarcodeType::EAN13:
                m_data = "123456789012";
                break;
            case BarcodeType::UPC_A:
                m_data = "12345678901";
                break;
            case BarcodeType::UPC_E:
                m_data = "123456";
                break;
            default:
                m_data = "12345678";
                break;
        }
    }

    generateBarcodeImage();
    setModified(true);
    emit typeChanged(type);
    emit itemChanged();
}

BarcodeType BarcodeItem::type() const
{
    return m_type;
}

void BarcodeItem::setForegroundColor(const QColor &color)
{
    if (m_foregroundColor == color) {
        return;
    }

    m_foregroundColor = color;
    generateBarcodeImage();
    setModified(true);
    emit foregroundColorChanged(color);
    emit itemChanged();
}

QColor BarcodeItem::foregroundColor() const
{
    return m_foregroundColor;
}

void BarcodeItem::setBackgroundColor(const QColor &color)
{
    if (m_backgroundColor == color) {
        return;
    }

    m_backgroundColor = color;
    generateBarcodeImage();
    setModified(true);
    emit backgroundColorChanged(color);
    emit itemChanged();
}

QColor BarcodeItem::backgroundColor() const
{
    return m_backgroundColor;
}

void BarcodeItem::setShowText(bool show)
{
    if (m_showText == show) {
        return;
    }

    m_showText = show;
    generateBarcodeImage();
    setModified(true);
    emit showTextChanged(show);
    emit itemChanged();
}

bool BarcodeItem::showText() const
{
    return m_showText;
}

void BarcodeItem::setTextFont(const QFont &font)
{
    if (m_textFont == font) {
        return;
    }

    m_textFont = font;
    generateBarcodeImage();
    setModified(true);
    emit textFontChanged(font);
    emit itemChanged();
}

QFont BarcodeItem::textFont() const
{
    return m_textFont;
}

void BarcodeItem::setMargin(int margin)
{
    if (m_margin == margin) {
        return;
    }

    m_margin = margin;
    generateBarcodeImage();
    setModified(true);
    emit marginChanged(margin);
    emit itemChanged();
}

int BarcodeItem::margin() const
{
    return m_margin;
}

void BarcodeItem::setWidth(qreal width)
{
    if (qFuzzyCompare(m_rect.width(), width)) {
        return;
    }

    setSize(QSizeF(width, m_rect.height()));
}

qreal BarcodeItem::width() const
{
    return m_rect.width();
}

void BarcodeItem::setHeight(qreal height)
{
    if (qFuzzyCompare(m_rect.height(), height)) {
        return;
    }

    setSize(QSizeF(m_rect.width(), height));
}

qreal BarcodeItem::height() const
{
    return m_rect.height();
}

void BarcodeItem::setIncludeChecksum(bool include)
{
    if (m_includeChecksum == include) {
        return;
    }

    m_includeChecksum = include;
    generateBarcodeImage();
    setModified(true);
    emit includeChecksumChanged(include);
    emit itemChanged();
}

bool BarcodeItem::includeChecksum() const
{
    return m_includeChecksum;
}

QString BarcodeItem::getTypeName(BarcodeType type)
{
    return barcodeTypeNames.value(type, "Code 128");
}

BarcodeType BarcodeItem::getTypeFromName(const QString &name)
{
    return barcodeTypeNames.key(name, BarcodeType::Code128);
}

QList<BarcodeType> BarcodeItem::getAllTypes()
{
    return barcodeTypeNames.keys();
}

bool BarcodeItem::validateData(const QString &data, BarcodeType type)
{
    if (data.isEmpty()) {
        return false;
    }

    switch (type) {
        case BarcodeType::Code128:
            // Code 128 可以编码所有ASCII字符
            return !data.isEmpty() && data.length() <= 80;

        case BarcodeType::Code39:
            // Code 39 只能包含大写字母、数字和特定符号
            for (QChar c : data) {
                if (!code39Patterns.contains(c) && c != '*') {
                    return false;
                }
            }
            return !data.isEmpty() && data.length() <= 80;

        case BarcodeType::EAN8:
            // EAN-8 必须有7位数字（不含校验位）
            if (data.length() != 7 && data.length() != 8) {
                return false;
            }
            for (QChar c : data) {
                if (!c.isDigit()) {
                    return false;
                }
            }
            return true;

        case BarcodeType::EAN13:
            // EAN-13 必须有12位数字（不含校验位）
            if (data.length() != 12 && data.length() != 13) {
                return false;
            }
            for (QChar c : data) {
                if (!c.isDigit()) {
                    return false;
                }
            }
            return true;

        case BarcodeType::UPC_A:
            // UPC-A 必须有11位数字（不含校验位）
            if (data.length() != 11 && data.length() != 12) {
                return false;
            }
            for (QChar c : data) {
                if (!c.isDigit()) {
                    return false;
                }
            }
            return true;

        case BarcodeType::UPC_E:
            // UPC-E 必须有6位数字（不含校验位）
            if (data.length() != 6 && data.length() != 7 && data.length() != 8) {
                return false;
            }
            for (QChar c : data) {
                if (!c.isDigit()) {
                    return false;
                }
            }
            return true;

        // 其他条形码类型的验证规则...

        default:
            // 默认情况下至少要有内容
            return !data.isEmpty();
    }
}

QImage BarcodeItem::generateBarcode(const QString &data, BarcodeType type,
                                   int width, int height,
                                   const QColor &foreground,
                                   const QColor &background,
                                   bool includeText,
                                   const QFont &textFont,
                                   int margin,
                                   bool includeChecksum)
{
    // 创建图像
    QImage image(width, height, QImage::Format_RGB32);
    image.fill(background);

    // 创建绘图对象
    QPainter painter(&image);

    // 设置抗锯齿
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // 计算文本高度（如果显示文本）
    int textHeight = 0;
    if (includeText) {
        QFontMetrics fm(textFont);
        textHeight = fm.height() + 4; // 添加一些间距
    }

    // 计算条形码高度
    int barcodeHeight = height - margin * 2 - textHeight;

    // 根据条形码类型生成编码
    QList<bool> barcode;
    int barWidth = width - margin * 2;

    switch (type) {
        case BarcodeType::Code128:
            barcode = encodeCode128(data, barWidth, barcodeHeight, includeText, textFont, margin);
            break;

        case BarcodeType::Code39:
            barcode = encodeCode39(data, barWidth, barcodeHeight, includeText, textFont, margin, includeChecksum);
            break;

        // 其他条形码类型的编码...

        default:
            // 默认使用Code 128
            barcode = encodeCode128(data, barWidth, barcodeHeight, includeText, textFont, margin);
            break;
    }

    // 绘制条形码
    if (!barcode.isEmpty()) {
        int moduleWidth = barWidth / barcode.size();
        if (moduleWidth < 1) moduleWidth = 1;

        painter.setPen(Qt::NoPen);
        painter.setBrush(foreground);

        for (int i = 0; i < barcode.size(); ++i) {
            if (barcode[i]) {
                painter.drawRect(margin + i * moduleWidth, margin,
                                moduleWidth, barcodeHeight);
            }
        }

        // 绘制文本
        if (includeText) {
            painter.setPen(foreground);
            painter.setFont(textFont);
            painter.drawText(QRect(margin, margin + barcodeHeight, width - margin * 2, textHeight),
                           Qt::AlignCenter, data);
        }
    }

    return image;
}

void BarcodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_locked && event->button() == Qt::LeftButton) {
        // 显示数据输入对话框
        bool ok;
        QString newData = QInputDialog::getText(nullptr, tr("编辑条形码"),
                                               tr("条形码数据:"), QLineEdit::Normal,
                                               m_data, &ok);
        if (ok && !newData.isEmpty()) {
            if (validateData(newData, m_type)) {
                setData(newData);
            } else {
                QInputDialog::warning(nullptr, tr("无效数据"),
                                     tr("输入的数据对于所选条形码类型无效。"));
            }
        }

        event->accept();
    } else {
        LabelItem::mouseDoubleClickEvent(event);
    }
}

bool BarcodeItem::generateBarcodeImage()
{
    // 确保数据和尺寸有效
    if (m_data.isEmpty() || m_rect.width() < 10 || m_rect.height() < 10) {
        m_barcodeImage = QImage();
        return false;
    }

    // 生成条形码图像
    m_barcodeImage = generateBarcode(m_data, m_type,
                                    m_rect.width(), m_rect.height(),
                                    m_foregroundColor, m_backgroundColor,
                                    m_showText, m_textFont, m_margin,
                                    m_includeChecksum);

    // 更新视图
    update();
    return !m_barcodeImage.isNull();
}

int BarcodeItem::calculateEANChecksum(const QString &data)
{
    int sum = 0;
    int multiplier;

    // EAN/UPC 校验位计算
    for (int i = data.length() - 1; i >= 0; --i) {
        int digit = data[i].digitValue();

        // 交替乘以1和3
        multiplier = ((data.length() - 1 - i) % 2) == 0 ? 1 : 3;
        sum += digit * multiplier;
    }

    // 计算校验位
    int checksum = (10 - (sum % 10)) % 10;
    return checksum;
}

QList<bool> BarcodeItem::encodeCode128(const QString &data, int &width, int height,
                                      bool includeText, const QFont &textFont, int margin)
{
    // 这里是简化版的Code 128实现
    // 实际项目中应该使用更完整的库

    QList<bool> bars;

    // 添加起始符 (Code 128 B)
    for (int bit : code128Patterns[1]) {
        for (int i = 0; i < bit; ++i) {
            bars.append(bars.size() % 2 == 0);
        }
    }

    // 编码数据
    // 此处为简化版，实际编码更复杂
    for (QChar c : data) {
        int code = c.unicode();
        if (code >= 32 && code <= 126) {
            // 生成一些简单的图案（实际应该使用标准的Code 128编码表）
            for (int i = 0; i < 11; ++i) {
                bars.append((i + code) % 2 == 0);
            }
        }
    }

    // 添加校验位（简化版）
    for (int i = 0; i < 11; ++i) {
        bars.append(i % 2 == 0);
    }

    // 添加终止符
    for (int bit : code128Patterns[3]) {
        for (int i = 0; i < bit; ++i) {
            bars.append(bars.size() % 2 == 0);
        }
    }

    return bars;
}

QList<bool> BarcodeItem::encodeCode39(const QString &data, int &width, int height,
                                     bool includeText, const QFont &textFont, int margin,
                                     bool includeChecksum)
{
    QList<bool> bars;

    // Code 39 编码
    QString codeData = data;

    // 如果数据不以*开始和结束，添加*作为开始/结束符
    if (!codeData.startsWith('*')) {
        codeData = '*' + codeData;
    }
    if (!codeData.endsWith('*')) {
        codeData = codeData + '*';
    }

    // 计算校验位（如果需要）
    if (includeChecksum) {
        int sum = 0;
        // 计算除了开始和结束符外的字符的校验和
        for (int i = 1; i < codeData.length() - 1; ++i) {
            QChar c = codeData.at(i);
            // 在实际项目中，这里应该使用一个标准的Code 39校验和计算方法
            // 这里简化处理，使用字符的ASCII值
            sum += c.toLatin1();
        }

        // 校验位是和除以43的余数
        char checkChar = static_cast<char>(sum % 43 + '0');
        if (checkChar > '9') checkChar = static_cast<char>(checkChar - '9' - 1 + 'A');
        if (checkChar > 'Z') checkChar = static_cast<char>(checkChar - 'Z' - 1 + '-');

        // 在结束符之前插入校验位
        codeData.insert(codeData.length() - 1, QChar(checkChar));
    }

    // 编码每个字符
    for (QChar c : codeData) {
        QString pattern = code39Patterns.value(c, "");
        if (pattern.isEmpty()) continue;

        // 转换模式为条和空格
        for (QChar p : pattern) {
            if (p == 'b') {
                bars.append(true);  // 条
            } else {
                bars.append(false); // 空格
            }
        }

        // 在字符之间添加空格
        bars.append(false);
    }

    return bars;
}

// 以下是更多条形码类型的编码方法，应根据实际需求实现
// 例如 encodeEAN13, encodeUPC_A 等

// EAN-13编码方法示例
QList<bool> BarcodeItem::encodeEAN13(const QString &data, int &width, int height,
                                    bool includeText, const QFont &textFont, int margin)
{
    QList<bool> bars;

    // 实际项目中，这里应该实现标准的EAN-13编码算法
    // 这是一个简化的示例

    // EAN-13编码具有特定的模式和分区
    QString codeData = data;

    // 确保数据长度为12位（不含校验位）
    if (codeData.length() < 12) {
        codeData = codeData.rightJustified(12, '0');
    } else if (codeData.length() > 12) {
        codeData = codeData.left(12);
    }

    // 计算校验位
    int checksum = calculateEANChecksum(codeData);
    codeData += QString::number(checksum);

    // 添加起始模式
    bars.append(true);
    bars.append(false);
    bars.append(true);

    // 编码前一组数字
    for (int i = 0; i < 6; ++i) {
        int digit = codeData.at(i).digitValue();
        // 在实际项目中，这里应该使用标准的EAN-13编码表
        for (int j = 0; j < 7; ++j) {
            bars.append((j + digit) % 2 == 0);
        }
    }

    // 添加中间模式
    bars.append(false);
    bars.append(true);
    bars.append(false);
    bars.append(true);
    bars.append(false);

    // 编码后一组数字
    for (int i = 6; i < 12; ++i) {
        int digit = codeData.at(i).digitValue();
        // 在实际项目中，这里应该使用标准的EAN-13编码表
        for (int j = 0; j < 7; ++j) {
            bars.append((j + digit) % 2 != 0);
        }
    }

    // 添加结束模式
    bars.append(true);
    bars.append(false);
    bars.append(true);

    return bars;
}

// 其他条形码类型的编码方法...
// 以下是完整的条形码生成软件中应该实现的其他方法

// UPC-A编码方法
QList<bool> BarcodeItem::encodeUPC_A(const QString &data, int &width, int height,
                                    bool includeText, const QFont &textFont, int margin)
{
    // 实际项目中，这里应该实现标准的UPC-A编码算法
    // UPC-A与EAN-13类似，但编码规则略有不同

    // 简化实现，实际返回EAN13编码（作为示例）
    return encodeEAN13("0" + data, width, height, includeText, textFont, margin);
}

// EAN-8编码方法
QList<bool> BarcodeItem::encodeEAN8(const QString &data, int &width, int height,
                                   bool includeText, const QFont &textFont, int margin)
{
    QList<bool> bars;

    // 实际项目中，这里应该实现标准的EAN-8编码算法
    // 这是一个简化的示例

    QString codeData = data;

    // 确保数据长度为7位（不含校验位）
    if (codeData.length() < 7) {
        codeData = codeData.rightJustified(7, '0');
    } else if (codeData.length() > 7) {
        codeData = codeData.left(7);
    }

    // 计算校验位
    int checksum = calculateEANChecksum(codeData);
    codeData += QString::number(checksum);

    // 添加起始模式
    bars.append(true);
    bars.append(false);
    bars.append(true);

    // 编码前一组数字
    for (int i = 0; i < 4; ++i) {
        int digit = codeData.at(i).digitValue();
        // 在实际项目中，这里应该使用标准的EAN-8编码表
        for (int j = 0; j < 7; ++j) {
            bars.append((j + digit) % 2 == 0);
        }
    }

    // 添加中间模式
    bars.append(false);
    bars.append(true);
    bars.append(false);
    bars.append(true);
    bars.append(false);

    // 编码后一组数字
    for (int i = 4; i < 8; ++i) {
        int digit = codeData.at(i).digitValue();
        // 在实际项目中，这里应该使用标准的EAN-8编码表
        for (int j = 0; j < 7; ++j) {
            bars.append((j + digit) % 2 != 0);
        }
    }

    // 添加结束模式
    bars.append(true);
    bars.append(false);
    bars.append(true);

    return bars;
}

// Interleaved 2 of 5编码方法
QList<bool> BarcodeItem::encodeInterleaved2of5(const QString &data, int &width, int height,
                                              bool includeText, const QFont &textFont, int margin,
                                              bool includeChecksum)
{
    QList<bool> bars;

    // 实际项目中，这里应该实现标准的Interleaved 2 of 5编码算法
    // 这是一个简化的示例

    QString codeData = data;

    // Interleaved 2 of 5需要偶数位数字
    if (codeData.length() % 2 != 0) {
        codeData = "0" + codeData;
    }

    // 如果需要校验位且长度为偶数，需要添加前导0
    if (includeChecksum && codeData.length() % 2 == 0) {
        codeData = "0" + codeData;
    }

    // 计算校验位（如果需要）
    if (includeChecksum) {
        int sum = 0;
        for (int i = 0; i < codeData.length(); ++i) {
            int digit = codeData.at(i).digitValue();
            // 奇数位置乘以3，偶数位置乘以1
            sum += digit * (i % 2 == 0 ? 3 : 1);
        }

        int checkDigit = (10 - (sum % 10)) % 10;
        codeData += QString::number(checkDigit);
    }

    // 添加起始符
    bars.append(true);
    bars.append(true);
    bars.append(false);
    bars.append(false);

    // 交错编码数字对
    for (int i = 0; i < codeData.length(); i += 2) {
        int firstDigit = codeData.at(i).digitValue();
        int secondDigit = codeData.at(i + 1).digitValue();

        // 在实际项目中，这里应使用标准的Interleaved 2 of 5编码表
        // 这里是简化实现
        for (int j = 0; j < 5; ++j) {
            // 编码第一个数字的条
            bars.append(true);
            bars.append((j + firstDigit) % 2 == 0);

            // 编码第二个数字的空格
            bars.append(false);
            bars.append((j + secondDigit) % 2 == 0);
        }
    }

    // 添加结束符
    bars.append(true);
    bars.append(true);
    bars.append(false);

    return bars;
}