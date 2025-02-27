#ifndef LABELEDITVIEW_H
#define LABELEDITVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QList>
#include <QPointF>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>

class LabelDocument;
class LabelItem;

/**
 * @brief 标签编辑视图类
 *
 * 提供标签的编辑界面，处理用户交互
 */
class LabelEditView : public QGraphicsView
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit LabelEditView(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~LabelEditView();

    /**
     * @brief 设置文档
     * @param document 标签文档
     */
    void setDocument(LabelDocument *document);

    /**
     * @brief 获取文档
     * @return 标签文档
     */
    LabelDocument* document() const;

    /**
     * @brief 获取选中的元素
     * @return 选中的元素列表
     */
    QList<LabelItem*> selectedItems() const;

    /**
     * @brief 获取当前缩放比例
     * @return 缩放比例
     */
    qreal currentZoom() const;

    /**
     * @brief 放大视图
     */
    void zoomIn();

    /**
     * @brief 缩小视图
     */
    void zoomOut();

    /**
     * @brief 重置缩放
     */
    void zoomReset();

    /**
     * @brief 设置特定缩放级别
     * @param zoom 缩放比例
     */
    void setZoom(qreal zoom);

    /**
     * @brief 适应窗口大小
     */
    void zoomToFit();

    /**
     * @brief 选择所有元素
     */
    void selectAll();

    /**
     * @brief 取消选择所有元素
     */
    void deselectAll();

    /**
     * @brief 删除选中的元素
     */
    void deleteSelectedItems();

    /**
     * @brief 剪切选中的元素
     */
    void cutSelectedItems();

    /**
     * @brief 复制选中的元素
     */
    void copySelectedItems();

    /**
     * @brief 粘贴元素
     */
    void pasteItems();

    /**
     * @brief 添加文本元素
     * @param pos 位置，如果为空则使用视图中心
     * @return 添加的元素
     */
    LabelItem* addTextElement(const QPointF &pos = QPointF());

    /**
     * @brief 添加图像元素
     * @param imagePath 图像路径
     * @param pos 位置，如果为空则使用视图中心
     * @return 添加的元素
     */
    LabelItem* addImageElement(const QString &imagePath = QString(), const QPointF &pos = QPointF());

    /**
     * @brief 添加条形码元素
     * @param pos 位置，如果为空则使用视图中心
     * @return 添加的元素
     */
    LabelItem* addBarcodeElement(const QPointF &pos = QPointF());

    /**
     * @brief 添加二维码元素
     * @param pos 位置，如果为空则使用视图中心
     * @return 添加的元素
     */
    LabelItem* addQRCodeElement(const QPointF &pos = QPointF());

    /**
     * @brief 更新视图
     */
    void updateView();

signals:
    /**
     * @brief 选择改变信号
     */
    void selectionChanged();

    /**
     * @brief 鼠标位置改变信号
     * @param pos 当前鼠标位置
     */
    void mousePositionChanged(const QPointF &pos);

    /**
     * @brief 缩放改变信号
     * @param zoom 当前缩放比例
     */
    void zoomChanged(qreal zoom);

    /**
     * @brief 标尺标记改变信号
     * @param pos 当前标记位置
     */
    void rulerMarkerChanged(const QPointF &pos);

    /**
     * @brief 视图状态改变信号
     * @param message 状态消息
     */
    void viewStatusChanged(const QString &message);

protected:
    /**
     * @brief 调整大小事件处理
     * @param event 调整大小事件
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief 鼠标按下事件处理
     * @param event 鼠标事件
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief 鼠标移动事件处理
     * @param event 鼠标事件
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief 鼠标释放事件处理
     * @param event 鼠标事件
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * @brief 鼠标双击事件处理
     * @param event 鼠标事件
     */
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    /**
     * @brief 鼠标滚轮事件处理
     * @param event 鼠标滚轮事件
     */
    void wheelEvent(QWheelEvent *event) override;

    /**
     * @brief 键盘按键事件处理
     * @param event 键盘事件
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 绘制背景
     * @param painter 绘图设备
     * @param rect 绘制区域
     */
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    /**
     * @brief 绘制前景
     * @param painter 绘图设备
     * @param rect 绘制区域
     */
    void drawForeground(QPainter *painter, const QRectF &rect) override;

    /**
     * @brief 上下文菜单事件处理
     * @param event 上下文菜单事件
     */
    void contextMenuEvent(QContextMenuEvent *event) override;

    /**
     * @brief 拖拽进入事件处理
     * @param event 拖拽事件
     */
    void dragEnterEvent(QDragEnterEvent *event) override;

    /**
     * @brief 拖拽移动事件处理
     * @param event 拖拽事件
     */
    void dragMoveEvent(QDragMoveEvent *event) override;

    /**
     * @brief 拖拽放下事件处理
     * @param event 拖拽事件
     */
    void dropEvent(QDropEvent *event) override;

private:
    /**
     * @brief 初始化场景
     */
    void initScene();

    /**
     * @brief 更新标签显示
     */
    void updateLabelDisplay();

    /**
     * @brief 视图坐标转换为场景坐标
     * @param pos 视图坐标
     * @return 场景坐标
     */
    QPointF viewToScenePos(const QPointF &pos) const;

    /**
     * @brief 绘制网格
     * @param painter 绘图设备
     * @param rect 绘制区域
     */
    void drawGrid(QPainter *painter, const QRectF &rect) const;

    /**
     * @brief 绘制标尺
     * @param painter 绘图设备
     * @param rect 绘制区域
     */
    void drawRulers(QPainter *painter, const QRectF &rect) const;

    /**
     * @brief 显示上下文菜单
     * @param pos 菜单位置
     * @param items 选中的元素
     */
    void showContextMenu(const QPoint &pos, const QList<LabelItem*> &items);

    /**
     * @brief 处理元素拖放数据
     * @param mimeData MIME数据
     * @param pos 放下位置
     */
    void handleDroppedData(const QMimeData *mimeData, const QPointF &pos);

    /**
     * @brief 创建选择矩形
     * @param startPos 起始位置
     * @param endPos 结束位置
     */
    void createSelectionRect(const QPointF &startPos, const QPointF &endPos);

    /**
     * @brief 更新元素选择状态
     */
    void updateSelectionState();

    /**
     * @brief 从元素创建MIME数据
     * @param items 要创建MIME数据的元素
     * @return MIME数据
     */
    QMimeData* createMimeDataFromItems(const QList<LabelItem*> &items) const;

    /**
     * @brief 从MIME数据创建元素
     * @param mimeData MIME数据
     * @return 创建的元素列表
     */
    QList<LabelItem*> createItemsFromMimeData(const QMimeData *mimeData) const;

private:
    QGraphicsScene *m_scene;        ///< 图形场景
    LabelDocument *m_document;      ///< 标签文档
    qreal m_zoom;                   ///< 当前缩放比例
    bool m_gridVisible;             ///< 网格是否可见
    QPointF m_lastMousePos;         ///< 上一次鼠标位置
    QPointF m_selectionStart;       ///< 选择矩形起始位置
    bool m_selecting;               ///< 是否正在进行区域选择
    QGraphicsRectItem *m_selectionRect; ///< 选择矩形
    int m_gridSize;                 ///< 网格大小
    bool m_snapToGrid;              ///< 是否对齐网格
    bool m_rulersVisible;           ///< 标尺是否可见
    QPointF m_rulerMarker;          ///< 标尺标记位置
    QGraphicsItemGroup *m_selectedItemsGroup; ///< 选中元素组
    bool m_movingItems;             ///< 是否正在移动元素
    QPointF m_moveStart;            ///< 移动起始位置
};

#endif // LABELEDITVIEW_H