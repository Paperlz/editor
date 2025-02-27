#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include "labelitem.h"

#include <QPixmap>
#include <QImage>
#include <QColor>
#include <QString>

/**
 * @brief 图像元素类
 *
 * 用于在标签上显示图像的元素
 */
class ImageItem : public LabelItem
{
    Q_OBJECT

    // 图像属性
    Q_PROPERTY(QString imagePath READ imagePath WRITE setImagePath NOTIFY imagePathChanged)
    Q_PROPERTY(bool keepAspectRatio READ keepAspectRatio WRITE setKeepAspectRatio NOTIFY keepAspectRatioChanged)
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(bool grayScale READ grayScale WRITE setGrayScale NOTIFY grayScaleChanged)

public:
    /**
     * @brief 构造函数
     * @param parent 父项目
     */
    explicit ImageItem(QGraphicsItem *parent = nullptr);

    /**
     * @brief 含图像路径的构造函数
     * @param imagePath 图像文件路径
     * @param parent 父项目
     */
    ImageItem(const QString &imagePath, QGraphicsItem *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ImageItem() override;

    /**
     * @brief 获取元素类型
     * @return 元素类型
     */
    int type() const override { return ImageType; }

    // QGraphicsItem 接口实现
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    // 序列化接口实现
    void saveToXml(QDomElement &element) const override;
    bool loadFromXml(const QDomElement &element) override;
    QJsonObject toJson() const override;
    bool fromJson(const QJsonObject &json) override;

    /**
     * @brief 克隆图像元素
     * @return 图像元素的副本
     */
    LabelItem* clone() const override;

    /**
     * @brief 更新元素内容
     */
    void updateContent() override;

    // 图像属性访问器

    /**
     * @brief 设置图像文件路径
     * @param path 文件路径
     * @return 加载是否成功
     */
    bool setImagePath(const QString &path);

    /**
     * @brief 获取图像文件路径
     * @return 文件路径
     */
    QString imagePath() const;

    /**
     * @brief 设置图像数据
     * @param image 图像数据
     * @return 设置是否成功
     */
    bool setImage(const QImage &image);

    /**
     * @brief 获取图像数据
     * @return 图像数据
     */
    QImage image() const;

    /**
     * @brief 设置是否保持宽高比
     * @param keep 是否保持宽高比
     */
    void setKeepAspectRatio(bool keep);

    /**
     * @brief 获取是否保持宽高比
     * @return 是否保持宽高比
     */
    bool keepAspectRatio() const;

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
     * @brief 设置不透明度
     * @param opacity 不透明度 (0.0-1.0)
     */
    void setOpacity(qreal opacity);

    /**
     * @brief 获取不透明度
     * @return 不透明度
     */
    qreal opacity() const;

    /**
     * @brief 设置是否灰度显示
     * @param gray 是否灰度
     */
    void setGrayScale(bool gray);

    /**
     * @brief 获取是否灰度显示
     * @return 是否灰度
     */
    bool grayScale() const;

    /**
     * @brief 裁剪图像
     * @param rect 裁剪矩形
     */
    void cropImage(const QRectF &rect);

    /**
     * @brief 旋转图像
     * @param angle 旋转角度（度）
     */
    void rotateImage(qreal angle);

    /**
     * @brief 翻转图像
     * @param horizontal 是否水平翻转
     */
    void flipImage(bool horizontal);

    /**
     * @brief 调整图像亮度
     * @param brightness 亮度调整值 (-100 到 100)
     */
    void adjustBrightness(int brightness);

    /**
     * @brief 调整图像对比度
     * @param contrast 对比度调整值 (-100 到 100)
     */
    void adjustContrast(int contrast);

    /**
     * @brief 重置图像
     *
     * 恢复到原始图像
     */
    void resetImage();

protected:
    /**
     * @brief 处理大小变化
     *
     * 如果保持宽高比，调整尺寸时保持图像比例
     */
    void resize(qreal width, qreal height) override;

private:
    /**
     * @brief 应用图像效果
     *
     * 根据当前设置应用灰度、亮度和对比度等效果
     */
    void applyEffects();

    /**
     * @brief 保存原始图像
     *
     * 保存一份原始图像用于效果重置
     */
    void saveOriginalImage();

private:
    QString m_imagePath;        ///< 图像文件路径
    QImage m_originalImage;     ///< 原始图像
    QImage m_processedImage;    ///< 处理后的图像
    QPixmap m_pixmap;           ///< 显示用的像素图
    bool m_keepAspectRatio;     ///< 是否保持宽高比
    int m_borderWidth;          ///< 边框宽度
    QColor m_borderColor;       ///< 边框颜色
    qreal m_opacity;            ///< 不透明度
    bool m_grayScale;           ///< 是否灰度显示
    int m_brightness;           ///< 亮度调整
    int m_contrast;             ///< 对比度调整

signals:
    /**
     * @brief 图像路径改变信号
     * @param path 新路径
     */
    void imagePathChanged(const QString &path);

    /**
     * @brief 宽高比设置改变信号
     * @param keep 是否保持宽高比
     */
    void keepAspectRatioChanged(bool keep);

    /**
     * @brief 边框宽度改变信号
     * @param width 新宽度
     */
    void borderWidthChanged(int width);

    /**
     * @brief 边框颜色改变信号
     * @param color 新颜色
     */
    void borderColorChanged(const QColor &color);

    /**
     * @brief 不透明度改变信号
     * @param opacity 新不透明度
     */
    void opacityChanged(qreal opacity);

    /**
     * @brief 灰度设置改变信号
     * @param gray 是否灰度
     */
    void grayScaleChanged(bool gray);

    /**
     * @brief 图像处理完成信号
     */
    void imageProcessed();
};

/**
 * @brief 调整图像效果命令
 *
 * 用于撤销/重做图像效果调整操作
 */
class AdjustImageCommand : public QUndoCommand
{
public:
    /**
     * @brief 构造函数
     * @param item 图像元素
     * @param oldImage 旧图像
     * @param newImage 新图像
     * @param name 操作名称
     */
    AdjustImageCommand(ImageItem *item, const QImage &oldImage,
                       const QImage &newImage, const QString &name);

    /**
     * @brief 执行操作
     */
    void redo() override;

    /**
     * @brief 撤销操作
     */
    void undo() override;

private:
    ImageItem *m_item;     ///< 图像元素
    QImage m_oldImage;     ///< 旧图像
    QImage m_newImage;     ///< 新图像
};

#endif // IMAGEITEM_H