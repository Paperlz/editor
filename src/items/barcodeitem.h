#ifndef BARCODEITEM_H
#define BARCODEITEM_H

#include "labelitem.h"

#include <QColor>
#include <QString>
#include <QFont>

/**
 * @brief 条形码类型枚举
 */
enum class BarcodeType {
    Code128,    ///< Code 128
    Code39,     ///< Code 39
    Code93,     ///< Code 93
    EAN8,       ///< EAN-8
    EAN13,      ///< EAN-13
    UPC_A,      ///< UPC-A
    UPC_E,      ///< UPC-E
    MSI,        ///< MSI
    Interleaved2of5, ///< Interleaved 2 of 5
    ITF14,      ///< ITF-14
    Codabar     ///< Codabar
};

/**
 * @brief 条形码元素类
 *
 * 用于在标签上显示和编辑条形码的元素
 */
class BarcodeItem : public LabelItem
{
    Q_OBJECT

    // 条形码属性
    Q_PROPERTY(QString data READ data WRITE setData NOTIFY dataChanged)
    Q_PROPERTY(BarcodeType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor NOTIFY foregroundColorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(bool showText READ showText WRITE setShowText NOTIFY showTextChanged)
    Q_PROPERTY(QFont textFont READ textFont WRITE setTextFont NOTIFY textFontChanged)
    Q_PROPERTY(int margin READ margin WRITE setMargin NOTIFY marginChanged)
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(qreal height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(bool includeChecksum READ includeChecksum WRITE setIncludeChecksum NOTIFY includeChecksumChanged)

public:
    /**
     * @brief 构造函数
     * @param parent 父项目
     */
    explicit BarcodeItem(QGraphicsItem *parent = nullptr);

    /**
     * @brief 含数据的构造函数
     * @param data 条形码数据
     * @param type 条形码类型
     * @param parent 父项目
     */
    BarcodeItem(const QString &data, BarcodeType type = BarcodeType::Code128, QGraphicsItem *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~BarcodeItem() override;

    /**
     * @brief 获取元素类型
     * @return 元素类型
     */
    int type() const override { return BarcodeType; }

    // QGraphicsItem 接口实现
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    // 序列化接口实现
    void saveToXml(QDomElement &element) const override;
    bool loadFromXml(const QDomElement &element) override;
    QJsonObject toJson() const override;
    bool fromJson(const QJsonObject &json) override;

    /**
     * @brief 克隆条形码元素
     * @return 条形码元素的副本
     */
    LabelItem* clone() const override;

    /**
     * @brief 更新元素内容
     */
    void updateContent() override;

    // 条形码属性访问器

    /**
     * @brief 设置条形码数据
     * @param data 条形码数据
     */
    void setData(const QString &data);

    /**
     * @brief 获取条形码数据
     * @return 条形码数据
     */
    QString data() const;

    /**
     * @brief 设置条形码类型
     * @param type 条形码类型
     */
    void setType(BarcodeType type);

    /**
     * @brief 获取条形码类型
     * @return 条形码类型
     */
    BarcodeType type() const;

    /**
     * @brief 设置前景色
     * @param color 前景色
     */
    void setForegroundColor(const QColor &color);

    /**
     * @brief 获取前景色
     * @return 前景色
     */
    QColor foregroundColor() const;

    /**
     * @brief 设置背景色
     * @param color 背景色
     */
    void setBackgroundColor(const QColor &color);

    /**
     * @brief 获取背景色
     * @return 背景色
     */
    QColor backgroundColor() const;

    /**
     * @brief 设置是否显示文本
     * @param show 是否显示文本
     */
    void setShowText(bool show);

    /**
     * @brief 获取是否显示文本
     * @return 是否显示文本
     */
    bool showText() const;

    /**
     * @brief 设置文本字体
     * @param font 文本字体
     */
    void setTextFont(const QFont &font);

    /**
     * @brief 获取文本字体
     * @return 文本字体
     */
    QFont textFont() const;

    /**
     * @brief 设置边距
     * @param margin 边距
     */
    void setMargin(int margin);

    /**
     * @brief 获取边距
     * @return 边距
     */
    int margin() const;

    /**
     * @brief 设置宽度
     * @param width 宽度
     */
    void setWidth(qreal width);

    /**
     * @brief 获取宽度
     * @return 宽度
     */
    qreal width() const;

    /**
     * @brief 设置高度
     * @param height 高度
     */
    void setHeight(qreal height);

    /**
     * @brief 获取高度
     * @return 高度
     */
    qreal height() const;

    /**
     * @brief 设置是否包含校验和
     * @param include 是否包含校验和
     */
    void setIncludeChecksum(bool include);

    /**
     * @brief 获取是否包含校验和
     * @return 是否包含校验和
     */
    bool includeChecksum() const;

    /**
     * @brief 获取条形码类型名称
     * @param type 条形码类型
     * @return 类型名称
     */
    static QString getTypeName(BarcodeType type);

    /**
     * @brief 从类型名称获取条形码类型
     * @param name 类型名称
     * @return 条形码类型
     */
    static BarcodeType getTypeFromName(const QString &name);

    /**
     * @brief 获取所有条形码类型
     * @return 条形码类型列表
     */
    static QList<BarcodeType> getAllTypes();

    /**
     * @brief 验证条形码数据
     * @param data 条形码数据
     * @param type 条形码类型
     * @return 数据是否有效
     */
    static bool validateData(const QString &data, BarcodeType type);

    /**
     * @brief 生成条形码图像
     * @param data 条形码数据
     * @param type 条形码类型
     * @param width 宽度
     * @param height 高度
     * @param foreground 前景色
     * @param background 背景色
     * @param includeText 是否包含文本
     * @param textFont 文本字体
     * @param margin 边距
     * @param includeChecksum 是否包含校验和
     * @return 条形码图像
     */
    static QImage generateBarcode(const QString &data, BarcodeType type,
                                  int width, int height,
                                  const QColor &foreground = Qt::black,
                                  const QColor &background = Qt::white,
                                  bool includeText = true,
                                  const QFont &textFont = QFont("Arial", 10),
                                  int margin = 10,
                                  bool includeChecksum = true);

protected:
    /**
     * @brief 鼠标双击事件处理
     * @param event 事件对象
     */
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    /**
     * @brief 生成条形码图像
     * @return 生成是否成功
     */
    bool generateBarcodeImage();

    /**
     * @brief 计算EAN/UPC校验位
     * @param data 条形码数据
     * @return 校验位
     */
    static int calculateEANChecksum(const QString &data);

    /**
     * @brief 编码Code128
     * @param data 条形码数据
     * @param width 模块宽度
     * @param height 条形码高度
     * @param includeText 是否包含文本
     * @param textFont 文本字体
     * @param margin 边距
     * @return 编码数据
     */
    static QList<bool> encodeCode128(const QString &data, int &width, int height,
                                     bool includeText, const QFont &textFont, int margin);

    /**
     * @brief 编码Code39
     * @param data 条形码数据
     * @param width 模块宽度
     * @param height 条形码高度
     * @param includeText 是否包含文本
     * @param textFont 文本字体
     * @param margin 边距
     * @param includeChecksum 是否包含校验和
     * @return 编码数据
     */
    static QList<bool> encodeCode39(const QString &data, int &width, int height,
                                    bool includeText, const QFont &textFont, int margin,
                                    bool includeChecksum);

    // ... 可以添加其他条形码类型的编码方法

private:
    QString m_data;                 ///< 条形码数据
    BarcodeType m_type;             ///< 条形码类型
    QColor m_foregroundColor;       ///< 前景色
    QColor m_backgroundColor;       ///< 背景色
    bool m_showText;                ///< 是否显示文本
    QFont m_textFont;               ///< 文本字体
    int m_margin;                   ///< 边距
    bool m_includeChecksum;         ///< 是否包含校验和
    QImage m_barcodeImage;          ///< 条形码图像

signals:
    /**
     * @brief 数据改变信号
     * @param data 新数据
     */
    void dataChanged(const QString &data);

    /**
     * @brief 类型改变信号
     * @param type 新类型
     */
    void typeChanged(BarcodeType type);

    /**
     * @brief 前景色改变信号
     * @param color 新前景色
     */
    void foregroundColorChanged(const QColor &color);

    /**
     * @brief 背景色改变信号
     * @param color 新背景色
     */
    void backgroundColorChanged(const QColor &color);

    /**
     * @brief 显示文本设置改变信号
     * @param show 是否显示文本
     */
    void showTextChanged(bool show);

    /**
     * @brief 文本字体改变信号
     * @param font 新字体
     */
    void textFontChanged(const QFont &font);

    /**
     * @brief 边距改变信号
     * @param margin 新边距
     */
    void marginChanged(int margin);

    /**
     * @brief 宽度改变信号
     * @param width 新宽度
     */
    void widthChanged(qreal width);

    /**
     * @brief 高度改变信号
     * @param height 新高度
     */
    void heightChanged(qreal height);

    /**
     * @brief 校验和设置改变信号
     * @param include 是否包含校验和
     */
    void includeChecksumChanged(bool include);
};

#endif // BARCODEITEM_H