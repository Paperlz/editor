#ifndef LABELMODELS_H
#define LABELMODELS_H

#include <QObject>
#include <QList>
#include <QPrinter>
#include <QPointF>
#include <QSizeF>
#include <QUndoStack>
#include <QDomDocument>
#include <QJsonObject>
#include <QPainter>

class LabelItem;
class QGraphicsScene;

/**
 * @brief 标签文档类
 *
 * 管理标签的所有元素和属性
 */
class LabelDocument : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QPrinter::PageSize pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)
    Q_PROPERTY(QPageLayout::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(QSizeF customSize READ customSize WRITE setCustomSize NOTIFY customSizeChanged)
    Q_PROPERTY(int dpi READ dpi WRITE setDpi NOTIFY dpiChanged)
    Q_PROPERTY(QMarginsF margins READ margins WRITE setMargins NOTIFY marginsChanged)

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit LabelDocument(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~LabelDocument();

    /**
     * @brief 添加元素
     * @param item 要添加的元素
     */
    void addItem(LabelItem *item);

    /**
     * @brief 移除元素
     * @param item 要移除的元素
     */
    void removeItem(LabelItem *item);

    /**
     * @brief 获取元素列表
     * @return 元素列表
     */
    QList<LabelItem*> items() const;

    /**
     * @brief 通过ID获取元素
     * @param id 元素ID
     * @return 元素指针，如果未找到则返回nullptr
     */
    LabelItem* itemById(const QString &id) const;

    /**
     * @brief 通过名称获取元素
     * @param name 元素名称
     * @return 元素指针，如果未找到则返回nullptr
     */
    LabelItem* itemByName(const QString &name) const;

    /**
     * @brief 通过索引获取元素
     * @param index 元素索引
     * @return 元素指针，如果索引无效则返回nullptr
     */
    LabelItem* itemAt(int index) const;

    /**
     * @brief 获取元素数量
     * @return 元素数量
     */
    int itemCount() const;

    /**
     * @brief 清空所有元素
     */
    void clear();

    /**
     * @brief 将文档保存到XML文件
     * @param device 输出设备
     * @return 是否保存成功
     */
    bool saveToXml(QIODevice *device) const;

    /**
     * @brief 从XML文件加载文档
     * @param device 输入设备
     * @return 是否加载成功
     */
    bool loadFromXml(QIODevice *device);

    /**
     * @brief 将文档转换为JSON对象
     * @return JSON对象
     */
    QJsonObject toJson() const;

    /**
     * @brief 从JSON对象加载文档
     * @param json JSON对象
     * @return 是否加载成功
     */
    bool fromJson(const QJsonObject &json);

    /**
     * @brief 渲染文档
     * @param painter 绘图设备
     * @param rect 渲染区域
     */
    void render(QPainter *painter, const QRectF &rect) const;

    /**
     * @brief 导出为图像
     * @param size 图像大小
     * @return 导出的图像
     */
    QImage toImage(const QSize &size) const;

    /**
     * @brief 设置关联的场景
     * @param scene 图形场景
     */
    void setScene(QGraphicsScene *scene);

    /**
     * @brief 获取关联的场景
     * @return 图形场景
     */
    QGraphicsScene* scene() const;

    /**
     * @brief 设置撤销栈
     * @param undoStack 撤销栈
     */
    void setUndoStack(QUndoStack *undoStack);

    /**
     * @brief 获取撤销栈
     * @return 撤销栈
     */
    QUndoStack* undoStack() const;

    /**
     * @brief 设置页面大小
     * @param size 页面大小
     */
    void setPageSize(QPrinter::PageSize size);

    /**
     * @brief 获取页面大小
     * @return 页面大小
     */
    QPrinter::PageSize pageSize() const;

    /**
     * @brief 设置页面方向
     * @param orientation 页面方向
     */
    void setOrientation(QPageLayout::Orientation orientation);

    /**
     * @brief 获取页面方向
     * @return 页面方向
     */
    QPageLayout::Orientation orientation() const;

    /**
     * @brief 设置自定义大小
     * @param size 自定义大小
     */
    void setCustomSize(const QSizeF &size);

    /**
     * @brief 获取自定义大小
     * @return 自定义大小
     */
    QSizeF customSize() const;

    /**
     * @brief 设置DPI
     * @param dpi 分辨率（每英寸点数）
     */
    void setDpi(int dpi);

    /**
     * @brief 获取DPI
     * @return 分辨率
     */
    int dpi() const;

    /**
     * @brief 设置页面边距
     * @param margins 页面边距
     */
    void setMargins(const QMarginsF &margins);

    /**
     * @brief 获取页面边距
     * @return 页面边距
     */
    QMarginsF margins() const;

    /**
     * @brief 获取页面尺寸（基于当前设置）
     * @return 页面尺寸
     */
    QSizeF pageRealSize() const;

    /**
     * @brief 获取内容区域（减去边距）
     * @return 内容区域
     */
    QRectF contentRect() const;

    /**
     * @brief 标记文档已修改
     */
    void setModified();

    /**
     * @brief 检查文档是否已修改
     * @return 是否已修改
     */
    bool isModified() const;

    /**
     * @brief 重置修改状态
     */
    void resetModified();

    /**
     * @brief 创建并添加新元素
     * @param type 元素类型
     * @param pos 位置
     * @return 创建的元素
     */
    LabelItem* createItem(int type, const QPointF &pos = QPointF());

    /**
     * @brief 创建文本元素
     * @param text 文本内容
     * @param pos 位置
     * @return 创建的元素
     */
    LabelItem* createTextItem(const QString &text = QString(), const QPointF &pos = QPointF());

    /**
     * @brief 创建图像元素
     * @param imagePath 图像路径
     * @param pos 位置
     * @return 创建的元素
     */
    LabelItem* createImageItem(const QString &imagePath = QString(), const QPointF &pos = QPointF());

    /**
     * @brief 创建条形码元素
     * @param data 条形码数据
     * @param pos 位置
     * @return 创建的元素
     */
    LabelItem* createBarcodeItem(const QString &data = QString(), const QPointF &pos = QPointF());

    /**
     * @brief 创建二维码元素
     * @param data 二维码数据
     * @param pos 位置
     * @return 创建的元素
     */
    LabelItem* createQRCodeItem(const QString &data = QString(), const QPointF &pos = QPointF());

    /**
     * @brief 将元素上移
     * @param item 要移动的元素
     * @return 是否移动成功
     */
    bool moveItemUp(LabelItem *item);

    /**
     * @brief 将元素下移
     * @param item 要移动的元素
     * @return 是否移动成功
     */
    bool moveItemDown(LabelItem *item);

    /**
     * @brief 将元素移到顶层
     * @param item 要移动的元素
     * @return 是否移动成功
     */
    bool moveItemToTop(LabelItem *item);

    /**
     * @brief 将元素移到底层
     * @param item 要移动的元素
     * @return 是否移动成功
     */
    bool moveItemToBottom(LabelItem *item);

    /**
     * @brief 复制元素
     * @param item 要复制的元素
     * @return 复制的元素
     */
    LabelItem* cloneItem(const LabelItem *item);

signals:
    /**
     * @brief 文档修改信号
     */
    void documentModified();

    /**
     * @brief 元素添加信号
     * @param item 添加的元素
     */
    void itemAdded(LabelItem *item);

    /**
     * @brief 元素移除信号
     * @param item 移除的元素
     */
    void itemRemoved(LabelItem *item);

    /**
     * @brief 元素更改信号
     * @param item 更改的元素
     */
    void itemChanged(LabelItem *item);

    /**
     * @brief 页面大小更改信号
     * @param size 新页面大小
     */
    void pageSizeChanged(QPrinter::PageSize size);

    /**
     * @brief 页面方向更改信号
     * @param orientation 新页面方向
     */
    void orientationChanged(QPageLayout::Orientation orientation);

    /**
     * @brief 自定义大小更改信号
     * @param size 新自定义大小
     */
    void customSizeChanged(const QSizeF &size);

    /**
     * @brief DPI更改信号
     * @param dpi 新DPI值
     */
    void dpiChanged(int dpi);

    /**
     * @brief 边距更改信号
     * @param margins 新边距
     */
    void marginsChanged(const QMarginsF &margins);

private:
    /**
     * @brief 连接元素信号
     * @param item 要连接的元素
     */
    void connectItemSignals(LabelItem *item);

    /**
     * @brief 断开元素信号
     * @param item 要断开的元素
     */
    void disconnectItemSignals(LabelItem *item);

    /**
     * @brief 更新元素索引
     */
    void updateItemIndexes();

    /**
     * @brief 保存元素到XML
     * @param document XML文档
     * @param parent 父元素
     */
    void saveItemsToXml(QDomDocument &document, QDomElement &parent) const;

    /**
     * @brief 从XML加载元素
     * @param parent 父元素
     * @return 是否加载成功
     */
    bool loadItemsFromXml(const QDomElement &parent);

    QList<LabelItem*> m_items;                  ///< 元素列表
    QGraphicsScene *m_scene;                    ///< 关联的场景
    QUndoStack *m_undoStack;                    ///< 撤销栈
    QPrinter::PageSize m_pageSize;              ///< 页面大小
    QPageLayout::Orientation m_orientation;     ///< 页面方向
    QSizeF m_customSize;                        ///< 自定义大小
    int m_dpi;                                  ///< 分辨率
    QMarginsF m_margins;                        ///< 页面边距
    bool m_modified;                            ///< 是否已修改
};

/**
 * @brief 添加元素命令
 *
 * 用于撤销/重做添加元素操作
 */
class AddItemCommand : public QUndoCommand
{
public:
    /**
     * @brief 构造函数
     * @param document 文档
     * @param item 要添加的元素
     */
    AddItemCommand(LabelDocument *document, LabelItem *item);

    /**
     * @brief 析构函数
     */
    ~AddItemCommand();

    /**
     * @brief 执行操作
     */
    void redo() override;

    /**
     * @brief 撤销操作
     */
    void undo() override;

private:
    LabelDocument *m_document;  ///< 文档
    LabelItem *m_item;          ///< 元素
    bool m_ownsItem;            ///< 是否拥有元素（用于撤销时的释放）
};

/**
 * @brief 移除元素命令
 *
 * 用于撤销/重做移除元素操作
 */
class RemoveItemCommand : public QUndoCommand
{
public:
    /**
     * @brief 构造函数
     * @param document 文档
     * @param item 要移除的元素
     */
    RemoveItemCommand(LabelDocument *document, LabelItem *item);

    /**
     * @brief 析构函数
     */
    ~RemoveItemCommand();

    /**
     * @brief 执行操作
     */
    void redo() override;

    /**
     * @brief 撤销操作
     */
    void undo() override;

private:
    LabelDocument *m_document;  ///< 文档
    LabelItem *m_item;          ///< 元素
    int m_index;                ///< 原始索引
    bool m_ownsItem;            ///< 是否拥有元素（用于重做时的释放）
};

/**
 * @brief 移动元素层级命令
 *
 * 用于撤销/重做元素层级移动操作
 */
class ReorderItemCommand : public QUndoCommand
{
public:
    /**
     * @brief 构造函数
     * @param document 文档
     * @param item 要移动的元素
     * @param oldIndex 原始索引
     * @param newIndex 新索引
     */
    ReorderItemCommand(LabelDocument *document, LabelItem *item, int oldIndex, int newIndex);

    /**
     * @brief 执行操作
     */
    void redo() override;

    /**
     * @brief 撤销操作
     */
    void undo() override;

private:
    LabelDocument *m_document;  ///< 文档
    LabelItem *m_item;          ///< 元素
    int m_oldIndex;             ///< 原始索引
    int m_newIndex;             ///< 新索引
};

#endif // LABELMODELS_H