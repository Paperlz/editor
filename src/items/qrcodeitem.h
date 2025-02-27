#ifndef QRCODEITEM_H
#define QRCODEITEM_H

#include "labelitem.h"

#include <QColor>
#include <QString>
#include <QFont>

/**
 * @brief QR码错误校正级别枚举
 */
enum class QRErrorCorrectionLevel {
    Low,        ///< 低级别错误校正（约7%）
    Medium,     ///< 中级别错误校正（约15%）
    Quartile,   ///< 四分之一级别错误校正（约25%）
    High        ///< 高级别错误校正（约30%）
};

/**
 * @brief 二维码元素类
 *
 * 用于在标签上显示和编辑二维码的元素
 */
class QRCodeItem : public LabelItem
{
    Q_OBJECT

    // 二维码属性
    Q_PROPERTY(QString data READ data WRITE setData NOTIFY dataChanged)
    Q_PROPERTY(QRErrorCorrectionLevel errorCorrectionLevel READ errorCorrectionLevel WRITE setErrorCorrectionLevel NOTIFY errorCorrectionLevelChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor NOTIFY foregroundColorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(int margin READ margin WRITE setMargin NOTIFY marginChanged)
    Q_PROPERTY(int size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(bool quietZone READ quietZone WRITE setQuietZone NOTIFY quietZoneChanged)

public:
    /**
     * @brief 构造函数
     * @param parent 父项目
     */
    explicit QRCodeItem(QGraphicsItem *parent = nullptr);

    /**
     * @brief 含数据的构造函数
     * @param data 二维码数据
     * @param parent 父项目
     */
    QRCodeItem(const QString &data, QGraphicsItem *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~QRCodeItem() override;

    /**
     * @brief 获取元素类型
     * @return 元素类型
     */
    int type() const override { return QRCodeType; }

    // QGraphicsItem 接口实现
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    // 序列化接口实现
    void saveToXml(QDomElement &element) const override;
    bool loadFromXml(const QDomElement &element) override;
    QJsonObject toJson() const override;
    bool fromJson(const QJsonObject &json) override;

    /**
     * @brief 克隆二维码元素
     * @return 二维码元素的副本
     */
    LabelItem* clone() const override;

    /**
     * @brief 更新元素内容
     */
    void updateContent() override;

    // 二维码属性访问器

    /**
     * @brief 设置二维码数据
     * @param data 二维码数据
     */
    void setData(const QString &data);

    /**
     * @brief 获取二维码数据
     * @return 二维码数据
     */
    QString data() const;

    /**
     * @brief 设置错误校正级别
     * @param level 错误校正级别
     */
    void setErrorCorrectionLevel(QRErrorCorrectionLevel level);

    /**
     * @brief 获取错误校正级别
     * @return 错误校正级别
     */
    QRErrorCorrectionLevel errorCorrectionLevel() const;

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
     * @brief 设置尺寸
     * @param size 尺寸
     */
    void setSize(int size);

    /**
     * @brief 获取尺寸
     * @return 尺寸
     */
    int size() const;

    /**
     * @brief 设置是否包含安静区
     * @param quietZone 是否包含安静区
     */
    void setQuietZone(bool quietZone);

    /**
     * @brief 获取是否包含安静区
     * @return 是否包含安静区
     */
    bool quietZone() const;

    /**
     * @brief 获取错误校正级别名称
     * @param level 错误校正级别
     * @return 级别名称
     */
    static QString getErrorCorrectionLevelName(QRErrorCorrectionLevel level);

    /**
     * @brief 从名称获取错误校正级别
     * @param name 级别名称
     * @return 错误校正级别
     */
    static QRErrorCorrectionLevel getErrorCorrectionLevelFromName(const QString &name);

    /**
     * @brief 生成二维码图像
     * @param data 二维码数据
     * @param errorCorrectionLevel 错误校正级别
     * @param size 尺寸
     * @param margin 边距
     * @param foreground 前景色
     * @param background 背景色
     * @param quietZone 是否包含安静区
     * @return 二维码图像
     */
    static QImage generateQRCode(const QString &data,
                                QRErrorCorrectionLevel errorCorrectionLevel = QRErrorCorrectionLevel::Medium,
                                int size = 200,
                                int margin = 10,
                                const QColor &foreground = Qt::black,
                                const QColor &background = Qt::white,
                                bool quietZone = true);

protected:
    /**
     * @brief 鼠标双击事件处理
     * @param event 事件对象
     */
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    /**
     * @brief 生成二维码图像
     * @return 生成是否成功
     */
    bool generateQRCodeImage();

    /**
     * @brief 获取错误校正级别字符
     * @param level 错误校正级别
     * @return 级别字符
     */
    static char getErrorCorrectionLevelChar(QRErrorCorrectionLevel level);

private:
    QString m_data;                         ///< 二维码数据
    QRErrorCorrectionLevel m_errorLevel;    ///< 错误校正级别
    QColor m_foregroundColor;               ///< 前景色
    QColor m_backgroundColor;               ///< 背景色
    int m_margin;                           ///< 边距
    int m_size;                             ///< 尺寸
    bool m_quietZone;                       ///< 是否包含安静区
    QImage m_qrCodeImage;                   ///< 二维码图像

signals:
    /**
     * @brief 数据改变信号
     * @param data 新数据
     */
    void dataChanged(const QString &data);

    /**
     * @brief 错误校正级别改变信号
     * @param level 新级别
     */
    void errorCorrectionLevelChanged(QRErrorCorrectionLevel level);

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
     * @brief 边距改变信号
     * @param margin 新边距
     */
    void marginChanged(int margin);

    /**
     * @brief 尺寸改变信号
     * @param size 新尺寸
     */
    void sizeChanged(int size);

    /**
     * @brief 安静区设置改变信号
     * @param quietZone 是否包含安静区
     */
    void quietZoneChanged(bool quietZone);
};

#endif // QRCODEITEM_H