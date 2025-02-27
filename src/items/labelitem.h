#ifndef LABELITEM_H
#define LABELITEM_H

#include <QGraphicsItem>
#include <QObject>
#include <QRectF>
#include <QPainter>
#include <QDomElement>
#include <QJsonObject>
#include <QGraphicsSceneMouseEvent>
#include <QUndoCommand>

/**
 * @brief 标签元素基类
 *
 * 这是所有标签元素（文本、图像、条形码等）的基类，
 * 提供了通用的属性和行为。
 */
class LabelItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

    // 属性
    Q_PROPERTY(QPointF position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QSizeF size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(bool locked READ isLocked WRITE setLocked NOTIFY lockedChanged)
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
    /**
     * @brief 标签元素类型枚举
     */
    enum ItemType {
        TextType = UserType + 1,   ///< 文本元素
        ImageType = UserType + 2,  ///< 图像元素
        BarcodeType = UserType + 3, ///< 条形码元素
        QRCodeType = UserType + 4   ///< 二维码元素
    };

    /**
     * @brief 构造函数
     * @param parent 父项目
     */
    explicit LabelItem(QGraphicsItem *parent = nullptr);

    /**
     * @brief 析构函数
     */
    virtual ~LabelItem() override;

    // QGraphicsItem 接口

    /**
     * @brief 获取元素边界矩形
     * @return 边界矩形
     */
    QRectF boundingRect() const override;

    /**
     * @brief 绘制元素
     * @param painter 绘图对象
     * @param option 样式选项
     * @param widget 小部件
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    /**
     * @brief 判断点是否在图形内部
     * @param point 点坐标
     * @return 如果点在图形内部则返回true
     */
    bool contains(const QPointF &point) const override;

    // 序列化接口

    /**
     * @brief 将元素保存到XML元素
     * @param element XML元素
     */
    virtual void saveToXml(QDomElement &element) const = 0;

    /**
     * @brief 从XML元素加载元素
     * @param element XML元素
     * @return 加载是否成功
     */
    virtual bool loadFromXml(const QDomElement &element) = 0;

    /**
     * @brief 转换为JSON对象
     * @return JSON对象
     */
    virtual QJsonObject toJson() const = 0;

    /**
     * @brief 从JSON对象加载
     * @param json JSON对象
     * @return 加载是否成功
     */
    virtual bool fromJson(const QJsonObject &json) = 0;

    // 位置和大小属性

    /**
     * @brief 设置元素位置
     * @param pos 新位置
     */
    void setPosition(const QPointF &pos);

    /**
     * @brief 获取元素位置
     * @return 当前位置
     */
    QPointF position() const;

    /**
     * @brief 设置元素大小
     * @param size 新大小
     */
    void setSize(const QSizeF &size);

    /**
     * @brief 获取元素大小
     * @return 当前大小
     */
    QSizeF size() const;

    /**
     * @brief 设置元素旋转角度
     * @param angle 角度（度）
     */
    void setRotation(qreal angle);

    /**
     * @brief 获取元素旋转角度
     * @return 当前角度（度）
     */
    qreal rotation() const;

    // 状态属性

    /**
     * @brief 设置元素锁定状态
     * @param locked 是否锁定
     */
    void setLocked(bool locked);

    /**
     * @brief 判断元素是否锁定
     * @return 如果锁定则返回true
     */
    bool isLocked() const;

    /**
     * @brief 设置元素是否可见
     * @param visible 是否可见
     */
    void setVisible(bool visible);

    /**
     * @brief 判断元素是否可见
     * @return 如果可见则返回true
     */
    bool isVisible() const;

    // 标识属性

    /**
     * @brief 设置元素ID
     * @param id 元素ID
     */
    void setId(const QString &id);

    /**
     * @brief 获取元素ID
     * @return 元素ID
     */
    QString id() const;

    /**
     * @brief 设置元素名称
     * @param name 元素名称
     */
    void setName(const QString &name);

    /**
     * @brief 获取元素名称
     * @return 元素名称
     */
    QString name() const;

    /**
     * @brief 设置元素已修改
     * @param modified 是否已修改
     */
    void setModified(bool modified = true);

    /**
     * @brief 判断元素是否已修改
     * @return 如果已修改则返回true
     */
    bool isModified() const;

    // 操作方法

    /**
     * @brief 移动元素
     * @param dx X轴移动距离
     * @param dy Y轴移动距离
     */
    virtual void moveBy(qreal dx, qreal dy);

    /**
     * @brief 调整元素大小
     * @param width 新宽度
     * @param height 新高度
     */
    virtual void resize(qreal width, qreal height);

    /**
     * @brief 创建元素副本
     * @return 元素副本
     */
    virtual LabelItem* clone() const = 0;

    /**
     * @brief 更新元素内容
     *
     * 由子类实现以更新元素的内部内容
     */
    virtual void updateContent() = 0;

protected:
    // 鼠标事件处理
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

    /**
     * @brief 更新控制点位置
     */
    virtual void updateHandles();

    /**
     * @brief 获取控制点类型
     * @param point 点坐标
     * @return 控制点类型，-1表示不是控制点
     */
    int handleAtPosition(const QPointF &point) const;

    // 绘制辅助方法

    /**
     * @brief 绘制选中效果
     * @param painter 绘图对象
     */
    void drawSelection(QPainter *painter) const;

    /**
     * @brief 绘制控制点
     * @param painter 绘图对象
     */
    void drawHandles(QPainter *painter) const;

    // 成员变量
    QString m_id;             ///< 元素ID
    QString m_name;           ///< 元素名称
    QRectF m_rect;            ///< 元素矩形区域
    qreal m_rotation;         ///< 元素旋转角度
    bool m_locked;            ///< 元素是否锁定
    bool m_visible;           ///< 元素是否可见
    bool m_modified;          ///< 元素是否已修改
    bool m_hovered;           ///< 鼠标是否悬停在元素上

    // 拖动状态变量
    bool m_dragging;          ///< 是否正在拖动
    int m_activeHandle;       ///< 当前活动的控制点
    QPointF m_dragStartPos;   ///< 拖动起始位置
    QPointF m_lastPos;        ///< 上一次鼠标位置
    QRectF m_startRect;       ///< 调整大小开始时的矩形

    // 控制点常量
    static const int HandleTopLeft = 0;      ///< 左上角控制点
    static const int HandleTopMiddle = 1;    ///< 顶部中间控制点
    static const int HandleTopRight = 2;     ///< 右上角控制点
    static const int HandleMiddleLeft = 3;   ///< 左侧中间控制点
    static const int HandleMiddleRight = 4;  ///< 右侧中间控制点
    static const int HandleBottomLeft = 5;   ///< 左下角控制点
    static const int HandleBottomMiddle = 6; ///< 底部中间控制点
    static const int HandleBottomRight = 7;  ///< 右下角控制点
    static const int HandleRotate = 8;       ///< 旋转控制点

    static const int HandleSize = 8;        ///< 控制点大小
    static const int HandleSpace = 4;       ///< 控制点与元素边缘的间距
    static const int RotateHandleDistance = 20; ///< 旋转控制点距离

signals:
    /**
     * @brief 元素位置改变信号
     * @param pos 新位置
     */
    void positionChanged(const QPointF &pos);

    /**
     * @brief 元素大小改变信号
     * @param size 新大小
     */
    void sizeChanged(const QSizeF &size);

    /**
     * @brief 元素旋转角度改变信号
     * @param angle 新角度
     */
    void rotationChanged(qreal angle);

    /**
     * @brief 元素锁定状态改变信号
     * @param locked 是否锁定
     */
    void lockedChanged(bool locked);

    /**
     * @brief 元素ID改变信号
     * @param id 新ID
     */
    void idChanged(const QString &id);

    /**
     * @brief 元素名称改变信号
     * @param name 新名称
     */
    void nameChanged(const QString &name);

    /**
     * @brief 元素改变信号
     */
    void itemChanged();

    /**
     * @brief 元素被选中信号
     */
    void itemSelected();

    /**
     * @brief 元素被取消选中信号
     */
    void itemDeselected();
};

/**
 * @brief 移动标签元素命令
 *
 * 用于撤销/重做元素移动操作
 */
class MoveItemCommand : public QUndoCommand
{
public:
    /**
     * @brief 构造函数
     * @param item 标签元素
     * @param oldPos 旧位置
     * @param newPos 新位置
     */
    MoveItemCommand(LabelItem *item, const QPointF &oldPos, const QPointF &newPos);

    /**
     * @brief 执行操作
     */
    void redo() override;

    /**
     * @brief 撤销操作
     */
    void undo() override;

private:
    LabelItem *m_item;  ///< 标签元素
    QPointF m_oldPos;   ///< 旧位置
    QPointF m_newPos;   ///< 新位置
};

/**
 * @brief 调整标签元素大小命令
 *
 * 用于撤销/重做元素大小调整操作
 */
class ResizeItemCommand : public QUndoCommand
{
public:
    /**
     * @brief 构造函数
     * @param item 标签元素
     * @param oldRect 旧矩形
     * @param newRect 新矩形
     */
    ResizeItemCommand(LabelItem *item, const QRectF &oldRect, const QRectF &newRect);

    /**
     * @brief 执行操作
     */
    void redo() override;

    /**
     * @brief 撤销操作
     */
    void undo() override;

private:
    LabelItem *m_item;  ///< 标签元素
    QRectF m_oldRect;   ///< 旧矩形
    QRectF m_newRect;   ///< 新矩形
};

/**
 * @brief 旋转标签元素命令
 *
 * 用于撤销/重做元素旋转操作
 */
class RotateItemCommand : public QUndoCommand
{
public:
    /**
     * @brief 构造函数
     * @param item 标签元素
     * @param oldAngle 旧角度
     * @param newAngle 新角度
     */
    RotateItemCommand(LabelItem *item, qreal oldAngle, qreal newAngle);

    /**
     * @brief 执行操作
     */
    void redo() override;

    /**
     * @brief 撤销操作
     */
    void undo() override;

private:
    LabelItem *m_item;  ///< 标签元素
    qreal m_oldAngle;   ///< 旧角度
    qreal m_newAngle;   ///< 新角度
};

#endif // LABELITEM_H