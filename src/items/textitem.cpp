#include "textitem.h"

#include <QPainter>
#include <QTextOption>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QGraphicsScene>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QTextCursor>
#include <QDebug>
#include <QDomDocument>
#include <QJsonObject>
#include <QFontMetrics>

TextItem::TextItem(QGraphicsItem *parent)
    : LabelItem(parent)
    , m_text(tr("双击编辑文本"))
    , m_font(QFont("Arial", 12))
    , m_textColor(Qt::black)
    , m_backgroundColor(Qt::transparent)
    , m_alignment(Qt::AlignLeft | Qt::AlignTop)
    , m_wordWrap(true)
    , m_borderWidth(0)
    , m_borderColor(Qt::black)
    , m_textDocument(new QTextDocument(this))
    , m_isEditing(false)
{
    // 设置元素类型
    setFlag(QGraphicsItem::ItemIsFocusable, true);

    // 设置名称
    setName(tr("文本"));

    // 初始化文本文档
    updateContent();

    // 设置大小
    QSizeF size = sizeHint();
    setSize(size);
}

TextItem::~TextItem()
{
    delete m_textDocument;
}

void TextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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
    if (m_backgroundColor != Qt::transparent) {
        painter->fillRect(m_rect, m_backgroundColor);
    }

    // 绘制边框
    if (m_borderWidth > 0) {
        painter->setPen(QPen(m_borderColor, m_borderWidth));
        painter->drawRect(m_rect);
    }

    // 如果正在编辑，不绘制文本（交由系统绘制）
    if (!m_isEditing) {
        // 设置绘制矩形区域
        QRectF textRect = m_rect.adjusted(2, 2, -2, -2); // 添加小边距

        // 如果使用文本文档
        if (m_wordWrap || m_alignment != (Qt::AlignLeft | Qt::AlignTop)) {
            // 定位文本文档
            m_textDocument->setTextWidth(textRect.width());

            painter->translate(textRect.topLeft());
            m_textDocument->drawContents(painter);
        } else {
            // 直接绘制文本
            painter->setFont(m_font);
            painter->setPen(m_textColor);
            painter->drawText(textRect, m_text);
        }
    }

    // 恢复画家状态
    painter->restore();
}

void TextItem::saveToXml(QDomElement &element) const
{
    // 保存基本属性
    element.setAttribute("type", "text");
    element.setAttribute("id", m_id);
    element.setAttribute("name", m_name);
    element.setAttribute("x", m_rect.x());
    element.setAttribute("y", m_rect.y());
    element.setAttribute("width", m_rect.width());
    element.setAttribute("height", m_rect.height());
    element.setAttribute("rotation", m_rotation);
    element.setAttribute("locked", m_locked ? "true" : "false");
    element.setAttribute("visible", m_visible ? "true" : "false");

    // 保存文本属性
    QDomElement textElement = element.ownerDocument().createElement("text");
    textElement.appendChild(element.ownerDocument().createCDATASection(m_text));
    element.appendChild(textElement);

    // 保存字体属性
    QDomElement fontElement = element.ownerDocument().createElement("font");
    fontElement.setAttribute("family", m_font.family());
    fontElement.setAttribute("pointSize", m_font.pointSize());
    fontElement.setAttribute("bold", m_font.bold() ? "true" : "false");
    fontElement.setAttribute("italic", m_font.italic() ? "true" : "false");
    fontElement.setAttribute("underline", m_font.underline() ? "true" : "false");
    element.appendChild(fontElement);

    // 保存颜色属性
    QDomElement colorElement = element.ownerDocument().createElement("colors");
    colorElement.setAttribute("text", m_textColor.name());
    colorElement.setAttribute("background", m_backgroundColor.name());
    colorElement.setAttribute("border", m_borderColor.name());
    element.appendChild(colorElement);

    // 保存格式属性
    QDomElement formatElement = element.ownerDocument().createElement("format");
    formatElement.setAttribute("alignment", static_cast<int>(m_alignment));
    formatElement.setAttribute("wordWrap", m_wordWrap ? "true" : "false");
    formatElement.setAttribute("borderWidth", m_borderWidth);
    element.appendChild(formatElement);
}

bool TextItem::loadFromXml(const QDomElement &element)
{
    // 检查类型
    if (element.attribute("type") != "text") {
        return false;
    }

    // 加载基本属性
    m_id = element.attribute("id");
    m_name = element.attribute("name", tr("文本"));

    // 设置几何属性
    qreal x = element.attribute("x", "0").toDouble();
    qreal y = element.attribute("y", "0").toDouble();
    qreal width = element.attribute("width", "100").toDouble();
    qreal height = element.attribute("height", "50").toDouble();
    m_rect = QRectF(x, y, width, height);

    m_rotation = element.attribute("rotation", "0").toDouble();
    m_locked = element.attribute("locked") == "true";
    m_visible = element.attribute("visible", "true") == "true";

    // 加载文本属性
    QDomElement textElement = element.firstChildElement("text");
    if (!textElement.isNull()) {
        m_text = textElement.text();
    }

    // 加载字体属性
    QDomElement fontElement = element.firstChildElement("font");
    if (!fontElement.isNull()) {
        QString family = fontElement.attribute("family", "Arial");
        int pointSize = fontElement.attribute("pointSize", "12").toInt();
        bool bold = fontElement.attribute("bold") == "true";
        bool italic = fontElement.attribute("italic") == "true";
        bool underline = fontElement.attribute("underline") == "true";

        m_font = QFont(family, pointSize);
        m_font.setBold(bold);
        m_font.setItalic(italic);
        m_font.setUnderline(underline);
    }

    // 加载颜色属性
    QDomElement colorElement = element.firstChildElement("colors");
    if (!colorElement.isNull()) {
        m_textColor = QColor(colorElement.attribute("text", "#000000"));
        m_backgroundColor = QColor(colorElement.attribute("background", "transparent"));
        m_borderColor = QColor(colorElement.attribute("border", "#000000"));
    }

    // 加载格式属性
    QDomElement formatElement = element.firstChildElement("format");
    if (!formatElement.isNull()) {
        m_alignment = static_cast<Qt::Alignment>(formatElement.attribute("alignment", "1").toInt());
        m_wordWrap = formatElement.attribute("wordWrap", "true") == "true";
        m_borderWidth = formatElement.attribute("borderWidth", "0").toInt();
    }

    // 更新内容
    updateContent();

    return true;
}

QJsonObject TextItem::toJson() const
{
    QJsonObject json;

    // 基本属性
    json["type"] = "text";
    json["id"] = m_id;
    json["name"] = m_name;
    json["x"] = m_rect.x();
    json["y"] = m_rect.y();
    json["width"] = m_rect.width();
    json["height"] = m_rect.height();
    json["rotation"] = m_rotation;
    json["locked"] = m_locked;
    json["visible"] = m_visible;

    // 文本属性
    json["text"] = m_text;

    // 字体属性
    QJsonObject fontJson;
    fontJson["family"] = m_font.family();
    fontJson["pointSize"] = m_font.pointSize();
    fontJson["bold"] = m_font.bold();
    fontJson["italic"] = m_font.italic();
    fontJson["underline"] = m_font.underline();
    json["font"] = fontJson;

    // 颜色属性
    QJsonObject colorJson;
    colorJson["text"] = m_textColor.name();
    colorJson["background"] = m_backgroundColor.name();
    colorJson["border"] = m_borderColor.name();
    json["colors"] = colorJson;

    // 格式属性
    QJsonObject formatJson;
    formatJson["alignment"] = static_cast<int>(m_alignment);
    formatJson["wordWrap"] = m_wordWrap;
    formatJson["borderWidth"] = m_borderWidth;
    json["format"] = formatJson;

    return json;
}

bool TextItem::fromJson(const QJsonObject &json)
{
    // 检查类型
    if (json["type"].toString() != "text") {
        return false;
    }

    // 加载基本属性
    m_id = json["id"].toString();
    m_name = json["name"].toString(tr("文本"));

    // 设置几何属性
    qreal x = json["x"].toDouble();
    qreal y = json["y"].toDouble();
    qreal width = json["width"].toDouble(100);
    qreal height = json["height"].toDouble(50);
    m_rect = QRectF(x, y, width, height);

    m_rotation = json["rotation"].toDouble();
    m_locked = json["locked"].toBool();
    m_visible = json["visible"].toBool(true);

    // 加载文本属性
    m_text = json["text"].toString();

    // 加载字体属性
    QJsonObject fontJson = json["font"].toObject();
    if (!fontJson.isEmpty()) {
        QString family = fontJson["family"].toString("Arial");
        int pointSize = fontJson["pointSize"].toInt(12);
        bool bold = fontJson["bold"].toBool();
        bool italic = fontJson["italic"].toBool();
        bool underline = fontJson["underline"].toBool();

        m_font = QFont(family, pointSize);
        m_font.setBold(bold);
        m_font.setItalic(italic);
        m_font.setUnderline(underline);
    }

    // 加载颜色属性
    QJsonObject colorJson = json["colors"].toObject();
    if (!colorJson.isEmpty()) {
        m_textColor = QColor(colorJson["text"].toString("#000000"));
        m_backgroundColor = QColor(colorJson["background"].toString("transparent"));
        m_borderColor = QColor(colorJson["border"].toString("#000000"));
    }

    // 加载格式属性
    QJsonObject formatJson = json["format"].toObject();
    if (!formatJson.isEmpty()) {
        m_alignment = static_cast<Qt::Alignment>(formatJson["alignment"].toInt(1));
        m_wordWrap = formatJson["wordWrap"].toBool(true);
        m_borderWidth = formatJson["borderWidth"].toInt(0);
    }

    // 更新内容
    updateContent();

    return true;
}

LabelItem* TextItem::clone() const
{
    TextItem *clone = new TextItem();

    // 复制基本属性
    clone->m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    clone->m_name = m_name;
    clone->m_rect = m_rect;
    clone->m_rotation = m_rotation;
    clone->m_locked = m_locked;
    clone->m_visible = m_visible;

    // 复制文本属性
    clone->m_text = m_text;
    clone->m_font = m_font;
    clone->m_textColor = m_textColor;
    clone->m_backgroundColor = m_backgroundColor;
    clone->m_alignment = m_alignment;
    clone->m_wordWrap = m_wordWrap;
    clone->m_borderWidth = m_borderWidth;
    clone->m_borderColor = m_borderColor;

    // 更新内容
    clone->updateContent();

    return clone;
}

void TextItem::updateContent()
{
    // 更新文本文档
    m_textDocument->clear();
    m_textDocument->setDefaultFont(m_font);

    // 设置文本选项
    QTextOption option;
    option.setAlignment(m_alignment);
    option.setWrapMode(m_wordWrap ? QTextOption::WordWrap : QTextOption::NoWrap);
    m_textDocument->setDefaultTextOption(option);

    // 设置文本颜色
    QTextCursor cursor(m_textDocument);
    cursor.select(QTextCursor::Document);
    QTextCharFormat format;
    format.setForeground(m_textColor);
    cursor.mergeCharFormat(format);

    // 设置文本内容
    m_textDocument->setPlainText(m_text);

    // 调整文档大小
    adjustTextDocument();

    // 更新视图
    update();
}

void TextItem::setText(const QString &text)
{
    if (m_text == text) {
        return;
    }

    m_text = text;
    updateContent();
    setModified(true);
    emit textChanged(text);
    emit itemChanged();
}

QString TextItem::text() const
{
    return m_text;
}

void TextItem::setFont(const QFont &font)
{
    if (m_font == font) {
        return;
    }

    m_font = font;
    updateContent();
    setModified(true);
    emit fontChanged(font);
    emit itemChanged();
}

QFont TextItem::font() const
{
    return m_font;
}

void TextItem::setTextColor(const QColor &color)
{
    if (m_textColor == color) {
        return;
    }

    m_textColor = color;
    updateContent();
    setModified(true);
    emit textColorChanged(color);
    emit itemChanged();
}

QColor TextItem::textColor() const
{
    return m_textColor;
}

void TextItem::setBackgroundColor(const QColor &color)
{
    if (m_backgroundColor == color) {
        return;
    }

    m_backgroundColor = color;
    update();
    setModified(true);
    emit backgroundColorChanged(color);
    emit itemChanged();
}

QColor TextItem::backgroundColor() const
{
    return m_backgroundColor;
}

void TextItem::setAlignment(Qt::Alignment alignment)
{
    if (m_alignment == alignment) {
        return;
    }

    m_alignment = alignment;
    updateContent();
    setModified(true);
    emit alignmentChanged(alignment);
    emit itemChanged();
}

Qt::Alignment TextItem::alignment() const
{
    return m_alignment;
}

void TextItem::setWordWrap(bool wrap)
{
    if (m_wordWrap == wrap) {
        return;
    }

    m_wordWrap = wrap;
    updateContent();
    setModified(true);
    emit wordWrapChanged(wrap);
    emit itemChanged();
}

bool TextItem::wordWrap() const
{
    return m_wordWrap;
}

void TextItem::setBorderWidth(int width)
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

int TextItem::borderWidth() const
{
    return m_borderWidth;
}

void TextItem::setBorderColor(const QColor &color)
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

QColor TextItem::borderColor() const
{
    return m_borderColor;
}

QSizeF TextItem::sizeHint() const
{
    // 如果没有文本，返回默认大小
    if (m_text.isEmpty()) {
        return QSizeF(100, 50);
    }

    // 创建临时文本文档计算大小
    QTextDocument document;
    document.setDefaultFont(m_font);
    document.setPlainText(m_text);

    if (m_wordWrap) {
        // 如果允许换行，限制宽度并计算所需高度
        document.setTextWidth(m_rect.width() > 0 ? m_rect.width() : 200);
        return QSizeF(document.textWidth(), document.size().height());
    } else {
        // 如果不允许换行，计算文本的实际尺寸
        QFontMetricsF fm(m_font);
        return QSizeF(fm.horizontalAdvance(m_text) + 10, fm.height() + 10);
    }
}

void TextItem::startEditing()
{
    if (m_locked || m_isEditing) {
        return;
    }

    m_isEditing = true;
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setTextInteractionFlags(Qt::TextEditorInteraction);
    setFocus(Qt::MouseFocusReason);
    update();

    emit editingChanged(true);
}

void TextItem::finishEditing()
{
    if (!m_isEditing) {
        return;
    }

    QString newText = toPlainText();
    if (newText != m_text) {
        setText(newText);
    }

    m_isEditing = false;
    setFlag(QGraphicsItem::ItemIsMovable, !m_locked);
    setTextInteractionFlags(Qt::NoTextInteraction);
    clearFocus();
    update();

    emit editingChanged(false);
}

void TextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_locked && event->button() == Qt::LeftButton) {
        startEditing();
        event->accept();
    } else {
        LabelItem::mouseDoubleClickEvent(event);
    }
}

void TextItem::keyPressEvent(QKeyEvent *event)
{
    if (m_isEditing) {
        if (event->key() == Qt::Key_Escape) {
            // 取消编辑
            finishEditing();
            event->accept();
        } else if (event->key() == Qt::Key_Return && (event->modifiers() & Qt::ControlModifier)) {
            // 完成编辑
            finishEditing();
            event->accept();
        } else {
            // 处理文本编辑
            QGraphicsTextItem::keyPressEvent(event);
        }
    } else {
        LabelItem::keyPressEvent(event);
    }
}

void TextItem::focusOutEvent(QFocusEvent *event)
{
    if (m_isEditing) {
        finishEditing();
    }

    LabelItem::focusOutEvent(event);
}

void TextItem::adjustTextDocument()
{
    if (m_wordWrap) {
        m_textDocument->setTextWidth(m_rect.width() - 4); // 4为边距
    } else {
        m_textDocument->setTextWidth(-1); // 不限制宽度
    }
}

// ============ EditTextCommand 实现 ============

EditTextCommand::EditTextCommand(TextItem *item, const QString &oldText, const QString &newText)
    : QUndoCommand(QObject::tr("编辑文本 %1").arg(item->name()))
    , m_item(item)
    , m_oldText(oldText)
    , m_newText(newText)
{
}

void EditTextCommand::redo()
{
    m_item->setText(m_newText);
}

void EditTextCommand::undo()
{
    m_item->setText(m_oldText);
}