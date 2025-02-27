#ifndef TEXTITEM_H
#define TEXTITEM_H

#include "labelitem.h"

#include <QFont>
#include <QColor>
#include <QString>
#include <QTextDocument>

/**
 * @brief 文本元素类
 *
 * 用于在标签上显示和编辑文本内容的元素
 */
class TextItem : public LabelItem
{
    Q_OBJECT

    // 文本属性
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)
    Q_PROPERTY(bool wordWrap READ wordWrap WRITE setWordWrap NOTIFY wordWrapChanged)
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)

public:
    /**
     * @brief 构造函数
     * @param parent 父项目
     */
    explicit TextItem(QGraphicsItem *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~TextItem() override;

    /**
     * @brief 获取元素类型
     * @return 元素类型
     */
    int type() const override { return TextType; }

    // QGraphicsItem 接口实现
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    // 序列化接口实现
    void saveToXml(QDomElement &element) const override;
    bool loadFromXml(const QDomElement &element) override;
    QJsonObject toJson() const override;
    bool fromJson(const QJsonObject &json) override;

    /**
     * @brief 克隆文本元素
     * @return 文本元素的副本
     */
    LabelItem* clone() const override;

    /**
     * @brief 更新元素内容
     */
    void updateContent() override;

    // 文本属性访问器

    /**
     * @brief 设置文本内容
     * @param text 文本内容
     */
    void setText(const QString &text);

    /**
     * @brief 获取文本内容
     * @return 文本内容
     */
    QString text() const;

    /**
     * @brief 设置字体
     * @param font 字体
     */
    void setFont(const QFont &font);

    /**
     * @brief 获取字体
     * @return 字体
     */
    QFont font() const;

    /**
     * @brief 设置文本颜色
     * @param color 文本颜色
     */
    void setTextColor(const QColor &color);

    /**
     * @brief 获取文本颜色
     * @return 文本颜色
     */
    QColor textColor() const;

    /**
     * @brief 设置背景颜色
     * @param color 背景颜色
     */
    void setBackgroundColor(const QColor &color);

    /**
     * @brief 获取背景颜色
     * @return 背景颜色
     */
    QColor backgroundColor() const;

    /**
     * @brief 设置文本对齐方式
     * @param alignment 对齐方式
     */
    void setAlignment(Qt::Alignment alignment);

    /**
     * @brief 获取文本对齐方式
     * @return 对齐方式
     */
    Qt::Alignment alignment() const;

    /**
     * @brief 设置是否自动换行
     * @param wrap 是否自动换行
     */
    void setWordWrap(bool wrap);

    /**
     * @brief 获取是否自动换行
     * @return 是否自动换行
     */
    bool wordWrap() const;

    /**
     * @brief 设置边框宽度
     * @param width 边框宽度
     */
    void setBorderWidth(int width);

    /**
     * @brief 获取边框宽度
     * @return 边框宽度
     */
    int borderWidth() const;

    /**
     * @brief 设置边框颜色
     * @param color 边框颜色
     */
    void setBorderColor(const QColor &color);

    /**
     * @brief 获取边框颜色
     * @return 边框颜色
     */
    QColor borderColor() const;

    /**
     * @brief 获取推荐大小
     * @return 推荐大小
     */
    QSizeF sizeHint() const;

    /**
     * @brief 开始编辑
     */
    void startEditing();

    /**
     * @brief 结束编辑
     */
    void finishEditing();

protected:
    /**
     * @brief 鼠标双击事件处理
     * @param event 事件对象
     */
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 键盘按键事件处理
     * @param event 事件对象
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 焦点失去事件处理
     * @param event 事件对象
     */
    void focusOutEvent(QFocusEvent *event) override;

    /**
     * @brief 调整文本文档大小
     */
    void adjustTextDocument();

private:
    QString m_text;                  ///< 文本内容
    QFont m_font;                    ///< 字体
    QColor m_textColor;              ///< 文本颜色
    QColor m_backgroundColor;        ///< 背景颜色
    Qt::Alignment m_alignment;       ///< 对齐方式
    bool m_wordWrap;                 ///< 是否自动换行
    int m_borderWidth;               ///< 边框宽度
    QColor m_borderColor;            ///< 边框颜色
    QTextDocument *m_textDocument;   ///< 文本文档对象
    bool m_isEditing;                ///< 是否正在编辑

signals:
    /**
     * @brief 文本内容改变信号
     * @param text 新文本内容
     */
    void textChanged(const QString &text);

    /**
     * @brief 字体改变信号
     * @param font 新字体
     */
    void fontChanged(const QFont &font);

    /**
     * @brief 文本颜色改变信号
     * @param color 新文本颜色
     */
    void textColorChanged(const QColor &color);

    /**
     * @brief 背景颜色改变信号
     * @param color 新背景颜色
     */
    void backgroundColorChanged(const QColor &color);

    /**
     * @brief 对齐方式改变信号
     * @param alignment 新对齐方式
     */
    void alignmentChanged(Qt::Alignment alignment);

    /**
     * @brief 自动换行设置改变信号
     * @param wrap 是否自动换行
     */
    void wordWrapChanged(bool wrap);

    /**
     * @brief 边框宽度改变信号
     * @param width 新边框宽度
     */
    void borderWidthChanged(int width);

    /**
     * @brief 边框颜色改变信号
     * @param color 新边框颜色
     */
    void borderColorChanged(const QColor &color);

    /**
     * @brief 编辑状态改变信号
     * @param editing 是否正在编辑
     */
    void editingChanged(bool editing);
};

/**
 * @brief 编辑文本命令
 *
 * 用于撤销/重做文本编辑操作
 */
class EditTextCommand : public QUndoCommand
{
public:
    /**
     * @brief 构造函数
     * @param item 文本元素
     * @param oldText 旧文本
     * @param newText 新文本
     */
    EditTextCommand(TextItem *item, const QString &oldText, const QString &newText);

    /**
     * @brief 执行操作
     */
    void redo() override;

    /**
     * @brief 撤销操作
     */
    void undo() override;

private:
    TextItem *m_item;     ///< 文本元素
    QString m_oldText;    ///< 旧文本
    QString m_newText;    ///< 新文本
};

#endif // TEXTITEM_H